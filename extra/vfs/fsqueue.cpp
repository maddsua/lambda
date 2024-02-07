#include "./formats.hpp"

#include <thread>

using namespace Lambda;
using namespace Lambda::VFS;
using namespace Lambda::VFS::Formats;

FSQueue& FSQueue::operator=(FSQueue&& other) noexcept {
	this->m_done = other.m_done;
	this->m_queue = std::move(other.m_queue);
	return *this;
}

void FSQueue::push(VirtualFile&& item) {
	std::lock_guard<std::mutex>lock(this->m_lock);
	this->m_queue.push(item);
}

VirtualFile FSQueue::next() {

	if (!this->m_queue.size()) {
		throw std::runtime_error("cannot get next item from an empty FSQueue");
	}

	std::lock_guard<std::mutex>lock(this->m_lock);

	VirtualFile temp = this->m_queue.front();
	this->m_queue.pop();

	return temp;
}

bool FSQueue::await() {

	auto readerDone = false;
	while (!this->m_done && !this->m_queue.size()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return this->m_queue.size();
}

bool FSQueue::done() const noexcept {
	return this->m_done;
}
