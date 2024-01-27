
#include "../http.hpp"
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

void HTTPServer::asyncReader(Network::TCP::Connection& conn, const HTTPTransportOptions& options, HttpRequestQueue& queue) {

	const auto& conninfo = conn.info();
	std::vector<uint8_t> recvBuff;
	bool connectionKeepAlive = false;

	do {

		auto headerEnded = recvBuff.end();
		while (conn.active() && headerEnded == recvBuff.end()) {

			auto newBytes = conn.read();
			if (!newBytes.size()) break;

			recvBuff.insert(recvBuff.end(), newBytes.begin(), newBytes.end());
			headerEnded = std::search(recvBuff.begin(), recvBuff.end(), patternEndHeader.begin(), patternEndHeader.end());
		}

		if (!recvBuff.size() || headerEnded == recvBuff.end()) break;

		auto headerFields = Strings::split(std::string(recvBuff.begin(), headerEnded), "\r\n");
		recvBuff.erase(recvBuff.begin(), headerEnded + patternEndHeader.size());

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
			next.request.url = "http://lambdahost:" + conninfo.hostPort + requestUrlString;
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

		if (options.reuseConnections) {
			auto connectionHeader = next.request.headers.get("connection");
			if (connectionKeepAlive) connectionKeepAlive = !Strings::includes(connectionHeader, "close");
				else connectionKeepAlive = Strings::includes(connectionHeader, "keep-alive");
			next.keepAlive = connectionKeepAlive;
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

			if (!conn.active()) {
				throw std::runtime_error("connection was terminated before request body could be received");
			}

			auto bodyRemaining = bodySize - recvBuff.size();
			if (bodyRemaining) {
				auto temp = conn.read(bodyRemaining);
				if (temp.size() != bodyRemaining) {
					throw std::runtime_error("connection terminated while receiving request body");
				}
				recvBuff.insert(recvBuff.end(), temp.begin(), temp.end());
			}

			next.request.body = std::vector<uint8_t>(recvBuff.begin(), recvBuff.begin() + bodySize);
			recvBuff.erase(recvBuff.begin(), recvBuff.begin() + bodySize);
		}

		queue.push(std::move(next));

	} while (conn.active() && connectionKeepAlive);
}

void HTTPServer::writeResponse(Lambda::HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings useEncoding) {

	#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

		std::vector<uint8_t> responseBody;

		switch (useEncoding) {

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

		if (useEncoding != ContentEncodings::None) {
			response.headers.set("content-encoding", contentEncodingMap.at(useEncoding));
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
