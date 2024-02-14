#include "./transport.hpp"
#include "./transport_impl.hpp"
#include "../polyfill/polyfill.hpp"
#include "../compression/compression.hpp"
#include "../network/network.hpp"

#include <set>
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;
using namespace Lambda::Network;

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

static const std::initializer_list<std::string> compressibleTypes = {
	"text", "html", "json", "xml", "javascript"
};

TransportContextV1::TransportContextV1(
	Network::TCP::Connection& connInit,
	const TransportOptions& optsInit
) : m_conn(connInit), m_topts(optsInit) {}

bool TransportContextV1::awaitNext() {

	if (this->m_keepalive == KeepAliveStatus::Close) {
		return this->m_next != nullptr;
	}

	if (this->m_next != nullptr) {
		return true;
	}

	static const std::string patternEndHeader = "\r\n\r\n";
	auto headerEnded = this->m_readbuff.end();

	while (this->m_conn.isOpen() && headerEnded == this->m_readbuff.end()) {

		auto newBytes = this->m_conn.read();
		if (!newBytes.size()) break;

		this->m_readbuff.insert(this->m_readbuff.end(), newBytes.begin(), newBytes.end());
		headerEnded = std::search(this->m_readbuff.begin(), this->m_readbuff.end(), patternEndHeader.begin(), patternEndHeader.end());

		if (this->m_readbuff.size() > this->m_topts.maxRequestSize) {
			throw ProtocolError("Request header too large", 413);
		}
	}

	if (!this->m_readbuff.size() || headerEnded == this->m_readbuff.end()) {
		return false;
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

		const auto connectionHeader = next.headers.get("connection");

		switch (this->m_keepalive) {

			case KeepAliveStatus::Unknown: {
				this->m_keepalive = (!connectionHeader.size() || Strings::includes(connectionHeader, "keep-alive")) ?
					KeepAliveStatus::KeepAlive : KeepAliveStatus::Close;
			} break;

			case KeepAliveStatus::KeepAlive: {
				if (Strings::includes(connectionHeader, "close"))
					this->m_keepalive = KeepAliveStatus::Close;
			} break;
			
			default: break;
		}
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
				throw NetworkError("Incomplete request body");
			}

			this->m_readbuff.insert(this->m_readbuff.end(), temp.begin(), temp.end());
		}

		next.body = std::vector<uint8_t>(this->m_readbuff.begin(), this->m_readbuff.begin() + bodySize);
		this->m_readbuff.erase(this->m_readbuff.begin(), this->m_readbuff.begin() + bodySize);
	}

	this->m_next = new IncomingRequest {
		std::move(next),
		Crypto::ShortID()
	};

	return true;
}

IncomingRequest TransportContextV1::nextRequest() {

	if (this->m_next == nullptr) {
		throw Lambda::Error("nextRequest() canceled: no requests pending. Use awaitNext() to read more requests");
	}

	const auto tempNext = std::move(*this->m_next);

	delete this->m_next;
	this->m_next = nullptr;

	return tempNext;
}

void TransportContextV1::respond(const ResponseContext& responsectx) {

	if (this->m_next != nullptr) {
		throw Lambda::Error("respond() canceled: Before responding to a request one must be read with nextRequest() call first");
	}

	auto applyEncoding = ContentEncodings::None;

	auto responseHeaders = responsectx.response.headers;
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

	std::vector<uint8_t> responseBodyBuffer;
	const auto& responseBody = responsectx.response.body.buffer();

	switch (applyEncoding) {

		case ContentEncodings::Brotli: {
			responseBodyBuffer = Compress::brotliCompressBuffer(responseBody, Compress::Quality::Noice);
		} break;

		case ContentEncodings::Gzip: {
			responseBodyBuffer = Compress::zlibCompressBuffer(responseBody, Compress::Quality::Noice, Compress::ZlibSetHeader::Gzip);
		} break;

		case ContentEncodings::Deflate: {
			responseBodyBuffer = Compress::zlibCompressBuffer(responseBody, Compress::Quality::Noice, Compress::ZlibSetHeader::Defalte);
		} break;

		default: {
			responseBodyBuffer = std::move(responseBody);
		} break;
	}

	const auto bodyBufferSize = responseBodyBuffer.size();

	if (applyEncoding != ContentEncodings::None) {
		responseHeaders.set("content-encoding", contentEncodingMap.at(applyEncoding));
	}

	if (this->flags.forceContentLength) {
		responseHeaders.set("content-length", std::to_string(bodyBufferSize));
	}

	responseHeaders.set("date", Date().toUTCString());
	responseHeaders.set("server", "maddsua/lambda");
	responseHeaders.set("x-request-id", this->m_id.toString() + '-' + responsectx.id.toString());

	//	set connection header to acknowledge keep-alive mode
	const auto responseConnectionHeader = responseHeaders.get("connection");
	if (responseConnectionHeader.size()) {
		this->m_keepalive = Strings::includes(responseConnectionHeader, "close") ?
			KeepAliveStatus::Close : KeepAliveStatus::KeepAlive;
	} else {
		const auto isKeepAlive = this->m_keepalive == KeepAliveStatus::KeepAlive;
		responseHeaders.set("connection", isKeepAlive ? "keep-alive" : "close");
	}

	std::vector<uint8_t> headerBuff;
	headerBuff.reserve(TCP::Connection::DefaultChunkSize);

	//	push http version
	static const std::string httpVersion = "HTTP/1.1";
	headerBuff.insert(headerBuff.end(), httpVersion.begin(), httpVersion.end());
	headerBuff.push_back(0x20);

	//	push response status code
	const auto& responseStatus = responsectx.response.status;
	const auto statusCode = std::to_string(responseStatus.code());
	headerBuff.insert(headerBuff.end(), statusCode.begin(), statusCode.end());
	headerBuff.push_back(0x20);

	//	push response status text
	const auto& statusText = responseStatus.text();
	headerBuff.insert(headerBuff.end(), statusText.begin(), statusText.end());

	static const std::string lineSeparator = "\r\n";
	headerBuff.insert(headerBuff.end(), lineSeparator.begin(), lineSeparator.end());

	//	serialize headers
	static const std::string headerSeparator = ": ";
	for (const auto& header : responseHeaders.entries()) {
		headerBuff.insert(headerBuff.end(), header.first.begin(), header.first.end());
		headerBuff.insert(headerBuff.end(), headerSeparator.begin(), headerSeparator.end());
		headerBuff.insert(headerBuff.end(), header.second.begin(), header.second.end());
		headerBuff.insert(headerBuff.end(), lineSeparator.begin(), lineSeparator.end());
	}

	//	add that empty line to indicate header end
	headerBuff.insert(headerBuff.end(), lineSeparator.begin(), lineSeparator.end());

	this->m_conn.write(headerBuff);
	if (bodyBufferSize) this->m_conn.write(responseBodyBuffer);

	//	close connection if keepalive mode is not in unknown or active state
	if (this->m_keepalive == KeepAliveStatus::Close) {
		this->m_conn.end();
	}
}

const std::string& TransportContextV1::contextID() const noexcept {
	return this->m_id.toString();
}

const Network::ConnectionInfo& TransportContextV1::conninfo() const noexcept {
	return this->m_conn.info();
}

const TransportOptions& TransportContextV1::options() const noexcept {
	return this->m_topts;
}

Network::TCP::Connection& TransportContextV1::tcpconn() const noexcept {
	return this->m_conn;
}

const ContentEncodings& TransportContextV1::getEnconding() const noexcept {
	return this->m_compress;
}

bool TransportContextV1::isConnected() const noexcept {
	return this->m_conn.isOpen();
}

void TransportContextV1::reset() noexcept {

	this->m_readbuff.clear();

	if (this->m_next != nullptr) {
		delete this->m_next;
		this->m_next = nullptr;
	}
}

bool TransportContextV1::hasPartialData() const noexcept {
	return this->m_readbuff.size() > 0;
}

void TransportContextV1::close() {
	this->m_conn.end();
}

void TransportContextV1::writeRaw(const std::vector<uint8_t>& data) {
	this->m_conn.write(data);
}

std::vector<uint8_t> TransportContextV1::readRaw() {
	return this->readRaw(Network::TCP::Connection::DefaultChunkSize);
}

std::vector<uint8_t> TransportContextV1::readRaw(size_t expectedSize) {

	auto bufferHave = std::move(this->m_readbuff);
	this->m_readbuff = {};

	if (bufferHave.size() < expectedSize) {
		auto bufferFetched = this->m_conn.read(expectedSize - bufferHave.size());
		bufferHave.insert(bufferHave.end(), bufferFetched.begin(), bufferFetched.end());
	}

	return bufferHave;
}
