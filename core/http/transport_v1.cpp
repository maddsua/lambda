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

const Network::ConnectionInfo& TransportContextV1::conninfo() const noexcept {
	return this->m_conn.info();
}

Network::TCP::Connection& TransportContextV1::getconn() noexcept {
	return this->m_conn;
}

const ContentEncodings& TransportContextV1::getEnconding() const noexcept {
	return this->m_compress;
}

TransportContextV1::TransportContextV1(
	Network::TCP::Connection& connInit,
	const TransportOptions& optsInit
) : m_conn(connInit), m_topts(optsInit) {}

std::optional<HTTP::Request> TransportContextV1::nextRequest() {

	auto headerEnded = this->m_readbuff.end();
	while (this->m_conn.active() && headerEnded == this->m_readbuff.end()) {

		auto newBytes = this->m_conn.read();
		if (!newBytes.size()) break;

		this->m_readbuff.insert(this->m_readbuff.end(), newBytes.begin(), newBytes.end());
		headerEnded = std::search(this->m_readbuff.begin(), this->m_readbuff.end(), patternEndHeader.begin(), patternEndHeader.end());

		if (this->m_readbuff.size() > this->m_topts.maxRequestSize) {
			throw ProtocolError("Request header too large", 413);
		}
	}

	if (!this->m_readbuff.size() || headerEnded == this->m_readbuff.end()) {
		return std::nullopt;
	}

	auto headerFields = Strings::split(std::string(this->m_readbuff.begin(), headerEnded), "\r\n");
	this->m_readbuff.erase(this->m_readbuff.begin(), headerEnded + patternEndHeader.size());

	auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	if (headerStartLine.size() < 2) {
		throw ProtocolError("Invalid HTTP request");
	}

	auto& requestMethodString = headerStartLine.at(0);
	auto& requestUrlString = headerStartLine.at(1);

	HTTP::Request next;
	next.method = HTTP::Method(requestMethodString);

	for (size_t i = 1; i < headerFields.size(); i++) {

		const auto& headerline = headerFields[i];

		const auto separator = headerline.find(':');
		if (separator == std::string::npos) {
			throw ProtocolError("Invalid request headers structure", 400);
		}

		const auto headerKey = Strings::trim(headerline.substr(0, separator));
		if (!headerKey.size()) {
			throw ProtocolError("Invalid request header (empty name)", 400);
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

	const auto& conninfo = this->m_conn.info();

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

	if (this->m_topts.reuseConnections) {
		auto connectionHeader = next.headers.get("connection");
		if (this->m_keepalive) this->m_keepalive = !Strings::includes(connectionHeader, "close");
			else this->m_keepalive = Strings::includes(connectionHeader, "keep-alive");
	}

	auto acceptEncodingHeader = next.headers.get("accept-encoding");
	if (this->m_topts.useCompression && acceptEncodingHeader.size()) {

		auto encodingNames = Strings::split(acceptEncodingHeader, ", ");
		auto acceptEncodingsSet = std::set<std::string>(encodingNames.begin(), encodingNames.end());

		for (const auto& encoding : contentEncodingMap) {
			if (acceptEncodingsSet.contains(encoding.second)) {
				this->m_compress = encoding.first;
				break;
			}
		}
	}

	auto bodySizeHeader = next.headers.get("content-length");
	size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

	if (bodySize) {

		const auto totalRequestSize = std::distance(this->m_readbuff.begin(), headerEnded) + bodySize;

		if (totalRequestSize > this->m_topts.maxRequestSize) {
			throw ProtocolError("Total request size is too large", 413);
		}

		auto bodyRemaining = bodySize - this->m_readbuff.size();
		if (bodyRemaining) {
	
			auto temp = this->m_conn.read(bodyRemaining);
			if (temp.size() != bodyRemaining) {
				throw ProtocolError("Incomplete request body");
			}

			this->m_readbuff.insert(this->m_readbuff.end(), temp.begin(), temp.end());
		}

		next.body = std::vector<uint8_t>(this->m_readbuff.begin(), this->m_readbuff.begin() + bodySize);
		this->m_readbuff.erase(this->m_readbuff.begin(), this->m_readbuff.begin() + bodySize);
	}

	return next;
}

void TransportContextV1::respond(const Response& response) {

	auto applyEncoding = ContentEncodings::None;

	auto responseHeaders = response.headers;
	const auto& contentTypeHeader = responseHeaders.get("content-type");

	if (this->flags.autocompress) {

		if (contentTypeHeader.size()) {

			auto contentTypeNormalized = Strings::toLowerCase(contentTypeHeader);

			//	when content type is provided, check if it's a text format,
			//	so that we won't be trying to compress jpegs and stuff
			for (const auto& item : compressibleTypes) {
				if (Strings::includes(contentTypeNormalized, item)) {
					applyEncoding = this->m_compress;
					break;
				}
			}

		} else {
			//	set content type in case it's not provided in response
			//	by default, it's assumed to be a html page. works fine with just text too
			responseHeaders.set("content-type", "text/html; charset=utf-8");
			applyEncoding = this->m_compress;
		}
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
			responseBody = std::move(response.body.buffer());
		} break;
	}

	const auto bodySize = responseBody.size();

	if (applyEncoding != ContentEncodings::None) {
		responseHeaders.set("content-encoding", contentEncodingMap.at(applyEncoding));
	}

	if (this->flags.forceContentLength) {
		responseHeaders.set("content-length", std::to_string(bodySize));
	}

	responseHeaders.set("date", Date().toUTCString());
	responseHeaders.set("server", "maddsua/lambda");

	//	set connection header to acknowledge keep-alive mode
	if (this->m_keepalive && !responseHeaders.has("connection")) {
		responseHeaders.set("connection", "keep-alive");
	}

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : responseHeaders.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	this->m_conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) this->m_conn.write(responseBody);
}

void TransportContextV1::reset() noexcept {
	this->m_readbuff.clear();
}

bool TransportContextV1::hasPartialData() const noexcept {
	return this->m_readbuff.size() > 0;
}
