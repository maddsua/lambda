
#include "../handlers.hpp"
#include "../../network/sysnetw.hpp"
#include "../../compression/compression.hpp"
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
using namespace Lambda::HTTPServer;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

static const std::initializer_list<std::string> compressibleTypes = {
	"text", "html", "json", "xml"
};

std::optional<RequestQueueItem> HTTPServer::requestReader(ReaderContext& ctx) {

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

	RequestQueueItem next;
	next.request.method = Lambda::HTTP::Method(requestMethodString);

	for (size_t i = 1; i < headerFields.size(); i++) {

		const auto& headerline = headerFields[i];

		auto separator = headerline.find(':');
		if (separator == std::string::npos) {
			throw std::runtime_error("invalid header structure (no separation betwen name and header value)");
		}

		auto headerKey = Strings::trim(headerline.substr(0, separator));
		if (!headerKey.size()) {
			throw std::runtime_error("invalid header (empty header name)");
		}

		auto headerValue = Strings::trim(headerline.substr(separator + 1));
		if (!headerValue.size()) {
			throw std::runtime_error("invalid header (empty header value)");
		}

		next.request.headers.append(headerKey, headerValue);
	}

	//	assemble request URL
	if (!requestUrlString.starts_with('/')) {
		throw std::runtime_error("invalid request URL");
	}

	auto hostHeader = next.request.headers.get("host");
	if (hostHeader.size()) {
		next.request.url = "http://" + hostHeader + requestUrlString;
	} else {
		next.request.url = "http://lambdahost:" + ctx.conninfo.hostPort + requestUrlString;
	}

	//	extract request url pathname
	size_t pathnameEndPos = std::string::npos;
	for (auto token : std::initializer_list<char>({ '?', '#' })) {
		auto tokenPos = next.pathname.find(token);
		if (tokenPos < pathnameEndPos)
			pathnameEndPos = tokenPos;
	}

	next.pathname = pathnameEndPos == std::string::npos ?
		requestUrlString :
		(pathnameEndPos ? requestUrlString.substr(0, pathnameEndPos) : "/");

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

void HTTPServer::asyncRequestReader(Network::TCP::Connection& conn, const HTTPTransportOptions& options, HttpRequestQueue& queue) {

	ReaderContext context {
		conn,
		options,
		conn.info()
	};

	do {

		auto next = requestReader(context);
		if (!next.has_value()) break;

		queue.push(std::move(next.value()));

	} while (conn.active() && context.keepAlive);
}

void HTTPServer::writeResponse(Lambda::HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings preferEncoding) {

	#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

		std::vector<uint8_t> responseBody;

		auto applyEncoding = ContentEncodings::None;
		auto responseContentType = Strings::toLowerCase(response.headers.get("content-type"));

		for (const auto& item : compressibleTypes) {
			if (Strings::includes(responseContentType, item)) {
				applyEncoding = preferEncoding;
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
			response.headers.set("content-encoding", contentEncodingMap.at(applyEncoding));
		}

	#else
		auto& responseBody = response.body.buffer();
	#endif

	auto bodySize = responseBody.size();
	response.headers.set("content-length", std::to_string(bodySize));

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : response.headers.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) conn.write(responseBody);

}
