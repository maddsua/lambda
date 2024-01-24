#include "./internal.hpp"

using namespace Lambda::Server;

RequestQueue::RequestQueue(const std::initializer_list<RequestQueueItem>& init) {
	this->m_queue = std::queue<RequestQueueItem>(init);
}

RequestQueue::RequestQueue(const Lambda::Server::RequestQueue& other) {
	this->m_done = other.m_done;
	this->m_queue = other.m_queue;
}

bool RequestQueue::hasNext() const noexcept {
	return this->m_queue.size() > 0;
}

void RequestQueue::push(const RequestQueueItem& item) {
	std::lock_guard<std::mutex>lock(this->m_mutex);
	this->m_queue.push(item);
}

void RequestQueue::finish() noexcept {
	this->m_done = true;
}

bool RequestQueue::await() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (this->m_queue.size() > 0) return true;
		else if (this->m_done) return false;
	}
}

RequestQueueItem RequestQueue::next() {

	std::lock_guard<std::mutex>lock(this->m_mutex);

	if (!this->hasNext())
		throw std::runtime_error("cannot get next item from an empty PipelineQueue");

	RequestQueueItem next = this->m_queue.front();
	this->m_queue.pop();

	return next;
}