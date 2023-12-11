#include "../network/sysnetw.hpp"
#include "../network.hpp"
#include "../server.hpp"
#include "../../core/polyfill.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>

using namespace Network;
using namespace Lambda;

static const std::string patternEndHeader = "\r\n\r\n";

struct PipelineItem {
	std::future<HTTP::Response> future;
	struct {
		bool keepAlive = false;
	} info;
};

void Lambda::handleHTTPConnection(TCPConnection& conn, HttpHandlerFunction handler) {

	std::queue<PipelineItem> pipeline;
	std::mutex pipelineMtLock;

	auto receiveInitedPromise = std::promise<void>();
	auto receiveInitedFuture = receiveInitedPromise.get_future();

	auto receiveLoop = std::async([&]() {

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

			auto headerStartLine = Strings::split(headerFields.at(0), " ");
			auto request = HTTP::Request(HTTP::URL(headerStartLine.at(1)));
			request.method = HTTP::Method(headerStartLine.at(0));

			for (size_t i = 1; i < headerFields.size(); i++) {

				const auto& headerline = headerFields[i];

				auto separator = headerline.find(':');
				if (separator == std::string::npos) throw std::runtime_error("header is fucked");

				auto headerKey = Strings::trim(headerline.substr(0, separator));
				if (!headerKey.size()) throw std::runtime_error("header key is fucked");

				auto headerValue = Strings::trim(headerline.substr(separator + 1));
				if (!headerValue.size()) throw std::runtime_error("header value is fucked");

				request.headers.append(headerKey, headerValue);
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

			auto connectionHeader = request.headers.get("connection");
			if (connectionKeepAlive) connectionKeepAlive = !Strings::includes(connectionHeader, "close");
				else connectionKeepAlive = Strings::includes(connectionHeader, "keep-alive");

			PipelineItem next;
			next.future = std::async(handler, std::move(request), conn.info());
			next.info.keepAlive = connectionKeepAlive;

			std::lock_guard<std::mutex>lock(pipelineMtLock);
			pipeline.push(std::move(next));

			if (receiveInitedFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
				receiveInitedPromise.set_value();
			}

		} while (connectionKeepAlive);

	});

	receiveInitedFuture.wait();

	while ((receiveLoop.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready || pipeline.size()) && conn.alive()) {

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

		auto bodySize = response.body.size();
		response.headers.set("content-length", std::to_string(bodySize));

		if (next.info.keepAlive) {
			response.headers.set("connection", "keep-alive");
		}

		std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
		for (const auto& header : response.headers.entries()) {
			headerBuff += header.key + ": " + header.value + "\r\n";
		}
		headerBuff += "\r\n";

		conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
		if (bodySize) conn.write(response.body.buffer());

		std::lock_guard<std::mutex>lock(pipelineMtLock);
		pipeline.pop();
	}
}
