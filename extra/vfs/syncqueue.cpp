#include "./formats.hpp"

#include <thread>

using namespace Lambda;
using namespace Lambda::VFS;
using namespace Lambda::VFS::Formats;

SyncQueue& SyncQueue::operator=(SyncQueue&& other) noexcept {
	this->m_done = other.m_done;
	this->m_queue = std::move(other.m_queue);
	return *this;
}

void SyncQueue::push(VirtualFile&& item) {
	std::lock_guard<std::mutex>lock(this->m_lock);
	this->m_queue.push(item);
}

VirtualFile SyncQueue::next() {

	if (!this->m_queue.size()) {
		throw std::runtime_error("cannot get next item from an empty FSQueue");
	}

	std::lock_guard<std::mutex>lock(this->m_lock);

	VirtualFile temp = this->m_queue.front();
	this->m_queue.pop();

	return temp;
}

bool SyncQueue::await() {

	while (!this->m_done && !this->m_queue.size()) {
		if (this->watchForExit != nullptr) {
			if (watchForExit->wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) break;
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	return this->m_queue.size();
}

bool SyncQueue::awaitEmpty() {

	while (!this->m_done && this->m_queue.size()) {
		if (this->watchForExit != nullptr) {
			if (watchForExit->wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) break;
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	return !this->m_queue.size();
}

void SyncQueue::close() noexcept {
	this->m_done = true;
}

bool SyncQueue::done() const noexcept {
	return this->m_done;
}

bool SyncQueue::empty() const noexcept {
	return this->m_queue.size() == 0;
}

void SyncQueue::setWatcher(std::future<void>* watch) {
	this->watchForExit = watch;
}
