#include "../internal.hpp"
#include "../../network/sysnetw.hpp"
#include "../../compression/compression.hpp"
#include "../../encoding/encoding.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../../build_options.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>
#include <map>
#include <set>

using namespace Lambda;
using namespace Lambda::Network;
using namespace Lambda::Server;
using namespace Lambda::Server::HTTPTransport;
using namespace Lambda::Server::Handlers;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

static const std::initializer_list<std::string> compressibleTypes = {
	"text", "html", "json", "xml"
};

std::optional<IncomingRequest> HTTPTransport::requestReader(HTTPReaderContext& ctx) {

	auto headerEnded = ctx.buffer.end();
	while (ctx.conn.active() && headerEnded == ctx.buffer.end()) {

		auto newBytes = ctx.conn.read();
		if (!newBytes.size()) break;

		ctx.buffer.insert(ctx.buffer.end(), newBytes.begin(), newBytes.end());
		headerEnded = std::search(ctx.buffer.begin(), ctx.buffer.end(), patternEndHeader.begin(), patternEndHeader.end());
	}

	if (!ctx.buffer.size() || headerEnded == ctx.buffer.end()) {
		return std::nullopt;
	}

	auto headerFields = Strings::split(std::string(ctx.buffer.begin(), headerEnded), "\r\n");
	ctx.buffer.erase(ctx.buffer.begin(), headerEnded + patternEndHeader.size());

	auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	if (headerStartLine.size() < 2) {
		throw std::runtime_error("invalid HTTP request");
	}

	auto& requestMethodString = headerStartLine.at(0);
	auto& requestUrlString = headerStartLine.at(1);

	IncomingRequest next;
	next.request.method = Lambda::HTTP::Method(requestMethodString);

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
			throw std::runtime_error("invalid header (empty header value)");
		}

		next.request.headers.append(headerKey, headerValue);
	}

	//	parse request url
	{

		const auto urlSearchPos = requestUrlString.find('?');
		//	I'll leave it here for redundancy
		const auto urlHashPos = requestUrlString.find('#', urlSearchPos != std::string::npos ? urlSearchPos : 0);
		const auto pathnameEndPos = std::min({ urlSearchPos, urlHashPos });

		next.request.url.pathname = pathnameEndPos == std::string::npos ?
			requestUrlString :
			(pathnameEndPos ? requestUrlString.substr(0, pathnameEndPos) : "/");

		if (urlSearchPos != std::string::npos) {
			auto trimSize = urlHashPos != std::string::npos ? (urlHashPos - urlSearchPos - 1) : 0;
			next.request.url.searchParams = trimSize ?
				requestUrlString.substr(urlSearchPos + 1, trimSize) :
				requestUrlString.substr(urlSearchPos + 1);
		}

		const auto hostHeader = next.request.headers.get("host");
		const auto hostHeaderSem = hostHeader.size() ? hostHeader.find(':') : std::string::npos;

		next.request.url.host = hostHeader.size() ? hostHeader : "lambdahost:" + std::to_string(ctx.conninfo.hostPort);

		next.request.url.hostname = hostHeader.size() ?
			(hostHeaderSem != std::string::npos ?
				hostHeader.substr(0, hostHeaderSem) : hostHeader) :
			"lambdahost";

		next.request.url.port = hostHeader.size() ?
			(hostHeaderSem != std::string::npos ?
				hostHeader.substr(hostHeaderSem + 1) : "") :
			std::to_string(ctx.conninfo.hostPort);

		const auto authHeader = next.request.headers.get("authorization");
		if (authHeader.size()) {
			auto basicAuthOpt = parseBasicAuth(authHeader);
			if (basicAuthOpt.has_value()) {
				next.request.url.username = basicAuthOpt.value().first;
				next.request.url.password = basicAuthOpt.value().second;
			}
		}
	}

	if (ctx.options.reuseConnections) {
		auto connectionHeader = next.request.headers.get("connection");
		if (ctx.keepAlive) ctx.keepAlive = !Strings::includes(connectionHeader, "close");
			else ctx.keepAlive = Strings::includes(connectionHeader, "keep-alive");
		next.keepAlive = ctx.keepAlive;
	}

	auto acceptEncodingHeader = next.request.headers.get("accept-encoding");
	if (acceptEncodingHeader.size()) {

		auto encodingNames = Strings::split(acceptEncodingHeader, ", ");
		auto acceptEncodingsSet = std::set<std::string>(encodingNames.begin(), encodingNames.end());

		for (const auto& encoding : contentEncodingMap) {
			if (acceptEncodingsSet.contains(encoding.second)) {
				next.acceptsEncoding = encoding.first;
				break;
			}
		}
	}

	auto bodySizeHeader = next.request.headers.get("content-length");
	size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

	if (bodySize) {

		if (!ctx.conn.active()) {
			throw std::runtime_error("connection was terminated before request body could be received");
		}

		auto bodyRemaining = bodySize - ctx.buffer.size();
		if (bodyRemaining) {
			auto temp = ctx.conn.read(bodyRemaining);
			if (temp.size() != bodyRemaining) {
				throw std::runtime_error("connection terminated while receiving request body");
			}
			ctx.buffer.insert(ctx.buffer.end(), temp.begin(), temp.end());
		}

		next.request.body = std::vector<uint8_t>(ctx.buffer.begin(), ctx.buffer.begin() + bodySize);
		ctx.buffer.erase(ctx.buffer.begin(), ctx.buffer.begin() + bodySize);
	}

	return next;
}

void HTTPTransport::writeResponse(const HTTP::Response& response, const HTTPWriterContext& ctx) {

	#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

		std::vector<uint8_t> responseBody;
		auto responseHeaders = response.headers;

		auto applyEncoding = ContentEncodings::None;
		auto responseContentType = Strings::toLowerCase(responseHeaders.get("content-type"));

		for (const auto& item : compressibleTypes) {
			if (Strings::includes(responseContentType, item)) {
				applyEncoding = ctx.acceptsEncoding;
				break;
			}
		}

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

	#else
		auto& responseBody = response.body.buffer();
	#endif

	auto bodySize = responseBody.size();
	responseHeaders.set("content-length", std::to_string(bodySize));
	responseHeaders.set("date", Date().toUTCString());
	responseHeaders.set("server", "maddsua/lambda");

	//	set connection header to acknowledge keep-alive mode
	if (ctx.keepAlive) {
		responseHeaders.set("connection", "keep-alive");
	}

	//	set content type in case it's not provided in response
	if (!response.headers.has("content-type")) {
		responseHeaders.set("content-type", "text/html; charset=utf-8");
	}

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : responseHeaders.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	ctx.conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) ctx.conn.write(responseBody);

}

std::optional<std::pair<std::string, std::string>> HTTPTransport::parseBasicAuth(const std::string& header) {

	if (!Strings::includes(Strings::toLowerCase(header), "basic")) {
		return std::nullopt;
	}

	const auto authStringStart = header.find_last_of(' ');
	if (authStringStart == std::string::npos) {
		return std::nullopt;
	}

	const auto authString = header.substr(authStringStart + 1);
	if (!authString.size()) {
		return std::nullopt;
	}

	const auto authStringDecoded = Encoding::fromBase64(authString);
	const auto authComponents = Strings::split(std::string(authStringDecoded.begin(), authStringDecoded.end()), ":");
	if (authComponents.size() < 2) {
		return std::nullopt;
	}

	return std::make_pair(authComponents[0], authComponents[1]);
}
