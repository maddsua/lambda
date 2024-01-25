#include <cstdio>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <memory>
#include <mutex>

struct Response {
	std::string data;
};

struct Request {
	std::string data;
};

class HttpRequestQueue {
	private:
		std::future<void> m_reader;
		std::queue<Response> m_queue;
		std::mutex m_lock;

	public:
		HttpRequestQueue();
		~HttpRequestQueue();

		HttpRequestQueue& operator=(const HttpRequestQueue& other) = delete;
		HttpRequestQueue& operator=(HttpRequestQueue&& other) noexcept;

		bool await();
		Response next();
		void push(const Response& item);
};

HttpRequestQueue::HttpRequestQueue() {
	this->m_reader = std::async([&](){
		for (size_t i = 0; i < 8; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			Response response;
			response.data = "response #" + std::to_string(i);
			this->m_queue.push(response);
		}
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

void HttpRequestQueue::push(const Response& item) {
	std::lock_guard<std::mutex>lock(this->m_lock);
	this->m_queue.push(item);
}

Response HttpRequestQueue::next() {
	if (!this->m_queue.size()) throw "empty queue";
	std::lock_guard<std::mutex>lock(this->m_lock);
	Response temp = this->m_queue.front();
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

int main(int argc, char const *argv[]) {

	auto incoming = HttpRequestQueue();

	while (incoming.await()) {
		auto next = incoming.next();
		puts(next.data.c_str());
	}
	
	return 0;
}
