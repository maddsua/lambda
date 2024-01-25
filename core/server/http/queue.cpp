#include <set>
#include <map>

#include "../http.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda::Server;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

HttpRequestQueue::HttpRequestQueue(Network::TCP::Connection& conn, const HTTPTransportOptions& options) {

	this->m_reader = std::async([&](){
		
		const auto& conninfo = conn.info();
		std::vector<uint8_t> recvBuff;
		bool connectionKeepAlive = false;

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
			recvBuff.erase(recvBuff.begin(), headerEnded + patternEndHeader.size());

			auto headerStartLine = Strings::split(headerFields.at(0), ' ');
	
			auto& requestMethodString = headerStartLine.at(0);
			auto& requestUrlString = headerStartLine.at(1);

			RequestQueueItem next;
			next.request.method = HTTP::Method(requestMethodString);

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

			//	construct request URL
			auto hostHeader = next.request.headers.get("host");
			if (hostHeader.size()) {
				next.request.url = HTTP::URL("http://" + hostHeader + requestUrlString);
			} else {
				next.request.url = HTTP::URL("http://lambdahost:" + conninfo.hostPort + requestUrlString);
			}

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

			//	unpack cookies
			auto cookieHeader = next.request.headers.get("cookie");
			if (cookieHeader.size()) {
				next.request.cookies = HTTP::Cookies(cookieHeader);
			}

			auto bodySizeHeader = next.request.headers.get("content-length");
			size_t bodySize = bodySizeHeader.size() ? std::stoull(bodySizeHeader) : 0;

			if (bodySize) {

				if (!conn.isOpen()) {
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

			this->push(std::move(next));

		} while (conn.isOpen() && connectionKeepAlive);
	});
}

HttpRequestQueue::~HttpRequestQueue() {
	if (this->m_reader.valid()) {
		try { this->m_reader.get(); }
			catch (...) {}
	}
}

HttpRequestQueue& HttpRequestQueue::operator=(HttpRequestQueue&& other) noexcept {
	this->m_reader = std::move(other.m_reader);
	this->m_queue = std::move(other.m_queue);
	return *this;
}

void HttpRequestQueue::push(RequestQueueItem&& item) {
	std::lock_guard<std::mutex>lock(this->m_lock);
	this->m_queue.push(item);
}

RequestQueueItem HttpRequestQueue::next() {

	if (!this->m_queue.size()) {
		throw std::runtime_error("cannot get next item from an empty HttpRequestQueue");
	}

	std::lock_guard<std::mutex>lock(this->m_lock);

	RequestQueueItem temp = this->m_queue.front();
	this->m_queue.pop();

	return temp;
}

bool HttpRequestQueue::await() {

	if (!m_reader.valid()) {
		return this->m_queue.size();
	}

	auto readerDone = false;
	while (!readerDone && !this->m_queue.size()) {
		readerDone = this->m_reader.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
	}

	if (readerDone) {
		this->m_reader.get();
	}

	return this->m_queue.size();
}
