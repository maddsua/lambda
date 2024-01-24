#include "../http.hpp"

using namespace Lambda::Server;

bool PipelineQueue::hasNext() const noexcept {
	return this->m_queue.size() > 0;
}

void PipelineQueue::push(const PipelineItem& item) {
	std::lock_guard<std::mutex>lock(this->m_mutex);
	this->m_queue.push(item);
}

void PipelineQueue::close() noexcept {
	this->m_done = true;
}

bool PipelineQueue::await() {
	while (!this->m_done && !this->m_queue.size()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return !this->m_done;
}

PipelineItem PipelineQueue::next() {

	std::lock_guard<std::mutex>lock(this->m_mutex);

	if (!this->hasNext())
		throw std::runtime_error("cannot get next item from an empty PipelineQueue");

	PipelineItem next = this->m_queue.front();
	this->m_queue.pop();

	return next;
}
