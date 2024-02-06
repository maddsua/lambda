#include "./transport.hpp"
#include "../polyfill/polyfill.hpp"
#include "../compression/compression.hpp"

#include <set>
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

static const std::initializer_list<std::string> compressibleTypes = {
	"text", "html", "json", "xml"
};

V1TransportContext::V1TransportContext(
	Network::TCP::Connection& connInit,
	const TransportOptions& optsInit
) : conn(connInit), opts(optsInit) {}

std::optional<HTTP::Request> V1TransportContext::nextRequest() {

	auto headerEnded = this->readbuffer.end();
	while (this->conn.active() && headerEnded == this->readbuffer.end()) {

		auto newBytes = this->conn.read();
		if (!newBytes.size()) break;

		this->readbuffer.insert(this->readbuffer.end(), newBytes.begin(), newBytes.end());
		headerEnded = std::search(this->readbuffer.begin(), this->readbuffer.end(), patternEndHeader.begin(), patternEndHeader.end());

		if (this->readbuffer.size() > this->opts.maxRequestSize) {
			throw std::runtime_error("request header size too big");
		}
	}

	if (!this->readbuffer.size() || headerEnded == this->readbuffer.end()) {
		return std::nullopt;
	}

	auto headerFields = Strings::split(std::string(this->readbuffer.begin(), headerEnded), "\r\n");
	this->readbuffer.erase(this->readbuffer.begin(), headerEnded + patternEndHeader.size());

	auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	if (headerStartLine.size() < 2) {
		throw std::runtime_error("invalid HTTP request");
	}

	auto& requestMethodString = headerStartLine.at(0);
	auto& requestUrlString = headerStartLine.at(1);

	HTTP::Request next;
	next.method = HTTP::Method(requestMethodString);

	for (size_t i = 1; i < headerFields.size(); i++) {

		const auto& headerline = headerFields[i];

		const auto separator = headerline.find(':');
		if (separator == std::string::npos) {
			throw std::runtime_error("invalid header structure (no separation betwen name and header value)");
		}

		const auto headerKey = Strings::trim(headerline.substr(0, separator));
		if (!headerKey.size()) {
			throw std::runtime_error("invalid header (empty header name)");
		}

		const auto headerValue = Strings::trim(headerline.substr(separator + 1));
		if (!headerValue.size()) {
			continue;
		}

		next.headers.append(headerKey, headerValue);
	}

	//	parse request url
	const auto urlSearchPos = requestUrlString.find('?');
	//	I'll leave it here for redundancy
	const auto urlHashPos = requestUrlString.find('#', urlSearchPos != std::string::npos ? urlSearchPos : 0);
	const auto pathnameEndPos = std::min({ urlSearchPos, urlHashPos });

	next.url.pathname = pathnameEndPos == std::string::npos ?
		requestUrlString :
		(pathnameEndPos ? requestUrlString.substr(0, pathnameEndPos) : "/");

	if (urlSearchPos != std::string::npos) {
		auto trimSize = urlHashPos != std::string::npos ? (urlHashPos - urlSearchPos - 1) : 0;
		next.url.searchParams = trimSize ?
			requestUrlString.substr(urlSearchPos + 1, trimSize) :
			requestUrlString.substr(urlSearchPos + 1);
	}

	const auto hostHeader = next.headers.get("host");
	const auto hostHeaderSem = hostHeader.size() ? hostHeader.find(':') : std::string::npos;

	const auto& conninfo = this->conn.info();

	next.url.host = hostHeader.size() ? hostHeader : "lambdahost:" + std::to_string(conninfo.hostPort);

	next.url.hostname = hostHeader.size() ?
		(hostHeaderSem != std::string::npos ?
			hostHeader.substr(0, hostHeaderSem) : hostHeader) :
		"lambdahost";

	next.url.port = hostHeader.size() ?
		(hostHeaderSem != std::string::npos ?
			hostHeader.substr(hostHeaderSem + 1) : "") :
		std::to_string(conninfo.hostPort);

	const auto authHeader = next.headers.get("authorization");
	if (authHeader.size()) {
		auto basicAuthOpt = HTTP::Auth::parseBasicAuth(authHeader);
		if (basicAuthOpt.has_value()) {
			const auto& creds = basicAuthOpt.value();
			next.url.username = creds.user;
			next.url.password = creds.password;
		}
	}

	if (this->opts.reuseConnections) {
		auto connectionHeader = next.headers.get("connection");
		if (this->keepAlive) this->keepAlive = !Strings::includes(connectionHeader, "close");
			else this->keepAlive = Strings::includes(connectionHeader, "keep-alive");
	}

	auto acceptEncodingHeader = next.headers.get("accept-encoding");
	if (this->opts.useCompression && acceptEncodingHeader.size()) {

		auto encodingNames = Strings::split(acceptEncodingHeader, ", ");
		auto acceptEncodingsSet = std::set<std::string>(encodingNames.begin(), encodingNames.end());

		for (const auto& encoding : contentEncodingMap) {
			if (acceptEncodingsSet.contains(encoding.second)) {
				this->acceptsEncoding = encoding.first;
				break;
			}
		}
	}

	auto bodySizeHeader = next.headers.get("content-length");
	size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

	if (bodySize) {

		const auto totalRequestSize = std::distance(this->readbuffer.begin(), headerEnded) + bodySize;

		if (totalRequestSize > this->opts.maxRequestSize) {
			throw std::runtime_error("total request size too big");
		}

		if (!this->conn.active()) {
			throw std::runtime_error("connection was terminated before request body could be received");
		}

		auto bodyRemaining = bodySize - this->readbuffer.size();
		if (bodyRemaining) {
			auto temp = this->conn.read(bodyRemaining);
			if (temp.size() != bodyRemaining) {
				throw std::runtime_error("connection terminated while receiving request body");
			}
			this->readbuffer.insert(this->readbuffer.end(), temp.begin(), temp.end());
		}

		next.body = std::vector<uint8_t>(this->readbuffer.begin(), this->readbuffer.begin() + bodySize);
		this->readbuffer.erase(this->readbuffer.begin(), this->readbuffer.begin() + bodySize);
	}

	return next;
}

void V1TransportContext::respond(const Response& response) {

	auto applyEncoding = ContentEncodings::None;

	auto responseHeaders = response.headers;
	const auto& contentTypeHeader = responseHeaders.get("content-type");

	if (contentTypeHeader.size()) {

		auto contentTypeNormalized = Strings::toLowerCase(contentTypeHeader);

		//	when content type is provided, check if it's a text format,
		//	so that we won't be trying to compress jpegs and stuff
		for (const auto& item : compressibleTypes) {
			if (Strings::includes(contentTypeNormalized, item)) {
				applyEncoding = this->acceptsEncoding;
				break;
			}
		}

	} else {
		//	set content type in case it's not provided in response
		//	by default, it's assumed to be a html page. works fine with just text too
		responseHeaders.set("content-type", "text/html; charset=utf-8");
		applyEncoding = this->acceptsEncoding;
	}

	std::vector<uint8_t> responseBody;

	switch (applyEncoding) {

		case ContentEncodings::Brotli: {
			responseBody = Compress::brotliCompressBuffer(response.body.buffer(), Compress::Quality::Noice);
		} break;

		case ContentEncodings::Gzip: {
			responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Gzip);
		} break;

		case ContentEncodings::Deflate: {
			responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Defalte);
		} break;

		default: {
			responseBody = response.body.buffer();
		} break;
	}

	if (applyEncoding != ContentEncodings::None) {
		responseHeaders.set("content-encoding", contentEncodingMap.at(applyEncoding));
	}

	auto bodySize = responseBody.size();
	responseHeaders.set("content-length", std::to_string(bodySize));
	responseHeaders.set("date", Date().toUTCString());
	responseHeaders.set("server", "maddsua/lambda");

	//	set connection header to acknowledge keep-alive mode
	if (this->keepAlive) {
		responseHeaders.set("connection", "keep-alive");
	}

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : responseHeaders.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	this->conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) this->conn.write(responseBody);
}
