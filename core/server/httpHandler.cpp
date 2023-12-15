#include "../server.hpp"
#include "../network/sysnetw.hpp"
#include "../compression.hpp"
#include "../polyfill.hpp"
#include "../crypto.hpp"
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
	uint32_t id;
	ContentEncodings acceptsEncoding = ContentEncodings::None;
	bool keepAlive = false;
};

void Server::handleHTTPConnection(TCPConnection&& conn, HttpHandlerFunction handler, const HttpHandlerOptions& options) {

	std::queue<PipelineItem> pipeline;
	std::mutex pipelineMutex;

	auto receiveRoutine = std::async([&]() {

		std::vector<uint8_t> recvBuff;

		bool connectionKeepAlive = false;
		size_t totalReadSize = 0;

		do {

			auto headerEnded = recvBuff.end();
			while (conn.isOpen() && headerEnded == recvBuff.end()) {

				auto newBytes = conn.read();
				if (!newBytes.size()) break;

				recvBuff.insert(recvBuff.end(), newBytes.begin(), newBytes.end());
				headerEnded = std::search(recvBuff.begin(), recvBuff.end(), patternEndHeader.begin(), patternEndHeader.end());
			}

			if (!recvBuff.size() || headerEnded == recvBuff.end()) break;

			auto headerFields = Strings::split(std::string(recvBuff.begin(), headerEnded), "\r\n");
			totalReadSize += recvBuff.size();
			recvBuff.erase(recvBuff.begin(), headerEnded + patternEndHeader.size());

			auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	
			auto& requestMethodString = headerStartLine.at(0);
			auto& requestUrlString = headerStartLine.at(1);

			PipelineItem next;
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
			next.keepAlive = connectionKeepAlive;

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

				if (!conn.isOpen()) throw std::runtime_error("connection was terminated before request body could be received");

				auto bodyRemaining = bodySize - recvBuff.size();
				if (bodyRemaining) {
					auto temp = conn.read(bodyRemaining);
					if (temp.size() != bodyRemaining) throw std::runtime_error("connection terminated while receiving request body");
					recvBuff.insert(recvBuff.end(), temp.begin(), temp.end());
				}

				next.request.body = std::vector<uint8_t>(recvBuff.begin(), recvBuff.begin() + bodySize);
				totalReadSize += recvBuff.size();
				recvBuff.erase(recvBuff.begin(), recvBuff.begin() + bodySize);
			}

			time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			next.id = (timeHighres & ~0UL) ^ (totalReadSize & ~0UL);

			std::lock_guard<std::mutex>lock(pipelineMutex);
			pipeline.push(std::move(next));

		} while (connectionKeepAlive);

	});

	while ((receiveRoutine.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready || pipeline.size()) && conn.isOpen()) {

		if (!pipeline.size()) continue;

		auto& next = pipeline.front();
		auto requestID = ShortID(next.id).toString();
		auto uinqueID = options.contextID.size() ? (options.contextID + '-' + requestID) : requestID;

		HTTP::Response response;
		auto responseDate = Date();

		try {

			response = handler(next.request, {
				uinqueID,
				conn.getInfo(),
				Console(requestID)
			});

		} catch(const std::exception& e) {

			if (options.loglevel.logRequests) {
				printf("%s [%s] [Caught error] Handler has crashed: %s\n", responseDate.toHRTString().c_str(), requestID.c_str(), e.what());
			}
			
			response = serviceResponse(500, std::string("Function handler crashed: ") + e.what());

		} catch(...) {

			if (options.loglevel.logRequests) {
				printf("%s [%s] [Caught error] Handler has crashed: unhandled exception\n", responseDate.toHRTString().c_str(), requestID.c_str());
			}

			response = serviceResponse(500, "Function handler crashed: unhandled exception");
		}

		response.headers.set("date", responseDate.toUTCString());
		response.headers.set("server", "maddsua/lambda");
		response.headers.set("x-request-id", uinqueID);
		if (next.keepAlive) response.headers.set("connection", "keep-alive");
		if (!response.headers.has("content-type")) response.headers.set("content-type", "text/html; charset=utf-8");

		#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

			std::vector<uint8_t> responseBody;

			switch (next.acceptsEncoding) {
	
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

			if (next.acceptsEncoding != ContentEncodings::None) {
				response.headers.set("content-encoding", contentEncodingMap.at(next.acceptsEncoding));
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

		if (options.loglevel.logRequests) {
			options.loglevel.logConnections ? 
				printf("%s %s %s %s --> %i\n",
					responseDate.toHRTString().c_str(),
					requestID.c_str(),
					static_cast<std::string>(next.request.method).c_str(),
					next.request.url.pathname.c_str(),
					response.status.code()
				) :
				printf("%s %s (%s) %s %s --> %i\n",
					responseDate.toHRTString().c_str(),
					requestID.c_str(),
					conn.getInfo().peerIP.c_str(),
					static_cast<std::string>(next.request.method).c_str(),
					next.request.url.pathname.c_str(),
					response.status.code()
				);
		}

		std::lock_guard<std::mutex>lock(pipelineMutex);
		pipeline.pop();
	}

	receiveRoutine.get();
}
