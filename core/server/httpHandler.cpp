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
	std::future<HTTP::Response> future;
	struct {
		bool keepAlive = false;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	} context;
};

void Server::handleHTTPConnection(TCPConnection conn, HttpHandlerFunction handler) {

	std::queue<PipelineItem> pipeline;
	std::mutex pipelineMtLock;

	auto receiveInitedPromise = std::promise<void>();
	auto receiveInitedFuture = receiveInitedPromise.get_future();

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
			auto request = HTTP::Request(HTTP::URL(headerStartLine.at(1)));
			request.method = HTTP::Method(headerStartLine.at(0));

			for (size_t i = 1; i < headerFields.size(); i++) {

				const auto& headerline = headerFields[i];

				auto separator = headerline.find(':');
				if (separator == std::string::npos) throw std::runtime_error("invalid header structure (no separation betwen name and header value)");

				auto headerKey = Strings::trim(headerline.substr(0, separator));
				if (!headerKey.size()) throw std::runtime_error("invalid header (empty header name)");

				auto headerValue = Strings::trim(headerline.substr(separator + 1));
				if (!headerValue.size()) throw std::runtime_error("invalid header (empty header value)");

				request.headers.append(headerKey, headerValue);
			}

			PipelineItem next;

			auto connectionHeader = request.headers.get("connection");
			if (connectionKeepAlive) connectionKeepAlive = !Strings::includes(connectionHeader, "close");
				else connectionKeepAlive = Strings::includes(connectionHeader, "keep-alive");
			next.context.keepAlive = connectionKeepAlive;

			auto acceptEncodingHeader = request.headers.get("accept-encoding");
			if (acceptEncodingHeader.size()) {

				auto encodingNames = Strings::split(acceptEncodingHeader, ", ");
				auto acceptEncodingsSet = std::set<std::string>(encodingNames.begin(), encodingNames.end());

				for (const auto& encoding : contentEncodingMap) {
					if (acceptEncodingsSet.contains(encoding.second)) {
						next.context.acceptsEncoding = encoding.first;
						break;
					}
				}
			}

			auto bodySizeHeader = request.headers.get("content-length");
			size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

			if (bodySize) {

				if (!conn.alive()) throw std::runtime_error("connection was terminated before request body could be received");

				auto bodyRemaining = bodySize - recvBuff.size();
				if (bodyRemaining) {
					auto temp = conn.read(bodyRemaining);
					if (temp.size() != bodyRemaining) throw std::runtime_error("connection terminated while receiving request body");
					recvBuff.insert(recvBuff.end(), temp.begin(), temp.end());
				}

				request.body = HTTP::Body(std::vector<uint8_t>(recvBuff.begin(), recvBuff.begin() + bodySize));
				recvBuff.erase(recvBuff.begin(), recvBuff.begin() + bodySize);
			}

			next.future = std::async(handler, std::move(request), conn.info());

			std::lock_guard<std::mutex>lock(pipelineMtLock);
			pipeline.push(std::move(next));

			if (receiveInitedFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
				receiveInitedPromise.set_value();
			}

		} while (connectionKeepAlive);

	});

	while (receiveInitedFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
		if (receiveRoutine.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			receiveRoutine.get();
	}

	while ((receiveRoutine.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready || pipeline.size()) && conn.alive()) {

		if (!pipeline.size()) continue;

		auto& next = pipeline.front();
		if (next.future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) continue;

		HTTP::Response response;

		try {
			response = next.future.get();
		} catch(const std::exception& e) {
			response = HTTP::Response(HTTP::Status(500), "function has crashed");
		} catch(...) {
			response = HTTP::Response(HTTP::Status(500), "function has crashed");
		}

		response.headers.set("date", Date().toUTCString());
		response.headers.set("server", "maddsua/lambda");
		if (next.context.keepAlive) response.headers.set("connection", "keep-alive");

		#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

			std::vector<uint8_t> responseBody;

			switch (next.context.acceptsEncoding) {
	
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

			if (next.context.acceptsEncoding != ContentEncodings::None) {
				response.headers.set("content-encoding", contentEncodingMap.at(next.context.acceptsEncoding));
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

		std::lock_guard<std::mutex>lock(pipelineMtLock);
		pipeline.pop();
	}

	receiveRoutine.get();
}
