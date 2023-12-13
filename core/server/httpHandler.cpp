#include "../server.hpp"
#include "../network/sysnetw.hpp"
#include "../compression.hpp"
#include "../../core/polyfill.hpp"
#include "../../lambda_build_options.hpp"

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

static const std::string patternEndHeader = "\r\n\r\n";

enum struct ContentEncodings {
	None = 0,
	Brotli = 1,
	Gzip = 2,
	Deflate = 3,
};

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{  ContentEncodings::Brotli, "br" },
	{  ContentEncodings::Gzip, "gzip" },
	{  ContentEncodings::Deflate, "deflate" },
};

struct PipelineItem {

	HTTP::Request request;

	struct {
		bool keepAlive = false;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	} ctx;

	std::string id;
};

void Server::handleHTTPConnection(TCPConnection&& conn, HttpHandlerFunction handler, const HttpHandlerOptions& options) {

	std::queue<PipelineItem> pipeline;
	std::mutex pipelineMtLock;

	auto receiveRoutine = std::async([&]() {

		std::vector<uint8_t> recvBuff;

		bool connectionKeepAlive = false;

		do {

			auto headerEnded = recvBuff.end();
			while (conn.alive() && headerEnded == recvBuff.end()) {

				auto newBytes = conn.read();
				if (!newBytes.size()) break;

				recvBuff.insert(recvBuff.end(), newBytes.begin(), newBytes.end());
				headerEnded = std::search(recvBuff.begin(), recvBuff.end(), patternEndHeader.begin(), patternEndHeader.end());
			}

			if (!recvBuff.size() || headerEnded == recvBuff.end()) break;

			auto headerFields = Strings::split(std::string(recvBuff.begin(), headerEnded), "\r\n");
			recvBuff.erase(recvBuff.begin(), headerEnded + patternEndHeader.size());

			auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	
			auto& requestMethodString = headerStartLine.at(0);
			auto& requestUrlString = headerStartLine.at(1);

			PipelineItem next;
			next.id = Crypto::randomID(8);
			next.request.url = HTTP::URL(requestUrlString);
			next.request.method = HTTP::Method(requestMethodString);

			for (size_t i = 1; i < headerFields.size(); i++) {

				const auto& headerline = headerFields[i];

				auto separator = headerline.find(':');
				if (separator == std::string::npos) throw std::runtime_error("invalid header structure (no separation betwen name and header value)");

				auto headerKey = Strings::trim(headerline.substr(0, separator));
				if (!headerKey.size()) throw std::runtime_error("invalid header (empty header name)");

				auto headerValue = Strings::trim(headerline.substr(separator + 1));
				if (!headerValue.size()) throw std::runtime_error("invalid header (empty header value)");

				next.request.headers.append(headerKey, headerValue);
			}

			auto connectionHeader = next.request.headers.get("connection");
			if (connectionKeepAlive) connectionKeepAlive = !Strings::includes(connectionHeader, "close");
				else connectionKeepAlive = Strings::includes(connectionHeader, "keep-alive");
			next.ctx.keepAlive = connectionKeepAlive;

			auto acceptEncodingHeader = next.request.headers.get("accept-encoding");
			if (acceptEncodingHeader.size()) {

				auto encodingNames = Strings::split(acceptEncodingHeader, ", ");
				auto acceptEncodingsSet = std::set<std::string>(encodingNames.begin(), encodingNames.end());

				for (const auto& encoding : contentEncodingMap) {
					if (acceptEncodingsSet.contains(encoding.second)) {
						next.ctx.acceptsEncoding = encoding.first;
						break;
					}
				}
			}

			auto bodySizeHeader = next.request.headers.get("content-length");
			size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

			if (bodySize) {

				if (!conn.alive()) throw std::runtime_error("connection was terminated before request body could be received");

				auto bodyRemaining = bodySize - recvBuff.size();
				if (bodyRemaining) {
					auto temp = conn.read(bodyRemaining);
					if (temp.size() != bodyRemaining) throw std::runtime_error("connection terminated while receiving request body");
					recvBuff.insert(recvBuff.end(), temp.begin(), temp.end());
				}

				next.request.body = std::vector<uint8_t>(recvBuff.begin(), recvBuff.begin() + bodySize);
				recvBuff.erase(recvBuff.begin(), recvBuff.begin() + bodySize);
			}

			std::lock_guard<std::mutex>lock(pipelineMtLock);
			pipeline.push(std::move(next));

		} while (connectionKeepAlive);

	});

	while ((receiveRoutine.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready || pipeline.size()) && conn.alive()) {

		if (!pipeline.size()) continue;

		auto& next = pipeline.front();

		HTTP::Response response;
		auto responseDate = Date();

		try {

			RequestContext requestCtx;
			requestCtx.requestID = next.id;
			requestCtx.conninfo = conn.info();
	
			response = handler(next.request, requestCtx);

		} catch(const std::exception& e) {

			if (options.errorLoggingEnabled) {
				printf("%s [%s] Handler has crashed: %s\n", responseDate.toHRTString().c_str(), next.id.c_str(), e.what());
			}

			response = HTTP::Response(HTTP::Status(500), "function has crashed");

		} catch(...) {

			if (options.errorLoggingEnabled) {
				printf("%s [%s] Handler has crashed: unhandled exception\n", responseDate.toHRTString().c_str(), next.id.c_str());
			}

			response = HTTP::Response(HTTP::Status(500), "function has crashed");
		}

		response.headers.set("date", responseDate.toUTCString());
		response.headers.set("server", "maddsua/lambda");
		response.headers.set("x-request-id", next.id);
		if (next.ctx.keepAlive) response.headers.set("connection", "keep-alive");

		#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

			std::vector<uint8_t> responseBody;

			switch (next.ctx.acceptsEncoding) {
	
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

			if (next.ctx.acceptsEncoding != ContentEncodings::None) {
				response.headers.set("content-encoding", contentEncodingMap.at(next.ctx.acceptsEncoding));
			}

		#else
			auto& responseBody = response.body.buffer();
		#endif

		auto bodySize = responseBody.size();
		response.headers.set("content-length", std::to_string(bodySize));

		std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
		for (const auto& header : response.headers.entries()) {
			headerBuff += header.key + ": " + header.value + "\r\n";
		}
		headerBuff += "\r\n";

		conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
		if (bodySize) conn.write(responseBody);

		if (options.reuqestLoggingEnabled) {
			auto conninfo = conn.info();
			printf("%s [%s] (%s) %s %s --> %i\n", responseDate.toHRTString().c_str(), next.id.c_str(), conninfo.ip.c_str(), static_cast<std::string>(next.request.method).c_str(), next.request.url.pathname.c_str(), response.status.code());
		}

		std::lock_guard<std::mutex>lock(pipelineMtLock);
		pipeline.pop();
	}

	receiveRoutine.get();
}
