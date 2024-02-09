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
	std::lock_guard lock(this->m_queue_lock);
	this->m_queue.push_back(item);
}

VirtualFile SyncQueue::next() {

	if (!this->m_queue.size()) {
		throw std::runtime_error("cannot get next item from an empty FSQueue");
	}

	std::lock_guard lock(this->m_queue_lock);

	VirtualFile temp = this->m_queue.front();
	this->m_queue.erase(this->m_queue.begin());

	return temp;
}

bool SyncQueue::await() {

	std::shared_lock lock(this->m_future_lock);

	while (!this->m_done && !this->m_queue.size()) {

		if (this->m_watch_future != nullptr) {
			if (m_watch_future->wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) break;
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	return this->m_queue.size();
}

bool SyncQueue::awaitEmpty() {

	std::shared_lock lock(this->m_future_lock);

	while (!this->m_done && this->m_queue.size()) {

		if (this->m_watch_future != nullptr) {
			if (m_watch_future->wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) break;
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

void SyncQueue::setPromiseExitWatcher(std::future<void>* watch) noexcept {
	std::unique_lock lock(this->m_future_lock);
	this->m_watch_future = watch;
}
