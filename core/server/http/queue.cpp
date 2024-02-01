#include <set>
#include <map>

#include "../handlers.hpp"

using namespace Lambda;
using namespace Lambda::HTTPServer;
using namespace Lambda::Server::Handlers;

HttpRequestQueue::HttpRequestQueue(Network::TCP::Connection& conn, const HTTPTransportOptions& options)
: ctx({ conn, options, conn.info() }) {

	this->m_reader = std::async([&](Network::TCP::Connection& conn, const HTTPTransportOptions& options) {

		do {

			auto next = requestReader(this->ctx);
			if (!next.has_value()) break;

			std::lock_guard<std::mutex>lock(this->m_lock);
			this->m_queue.push(std::move(next.value()));

		} while (conn.active() && this->ctx.keepAlive);

	}, std::ref(conn), std::ref(options));
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
