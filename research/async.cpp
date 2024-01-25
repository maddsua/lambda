#include <cstdio>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <memory>

struct Response {
	std::string data;
};

struct Request {
	std::string data;
};

class HttpRequestQueue {
	private:
		std::future<void> reader;
		std::queue<Response> queue;
	public:
		HttpRequestQueue();
		~HttpRequestQueue();

		HttpRequestQueue& operator= (const HttpRequestQueue& other) = delete;
		HttpRequestQueue& operator= (HttpRequestQueue&& other) noexcept;

		bool await();
		Response next();
		void push(const Response& item);
};

HttpRequestQueue::HttpRequestQueue() {
	this->reader = std::async([&](){
		for (size_t i = 0; i < 8; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			Response response;
			response.data = "response #" + std::to_string(i);
			this->queue.push(response);
		}
	});
}

HttpRequestQueue::~HttpRequestQueue() {
	if (this->reader.valid()) {
		try { this->reader.get(); }
			catch (...) {}
	}
}

HttpRequestQueue& HttpRequestQueue::operator= (HttpRequestQueue&& other) noexcept {
	this->reader = std::move(other.reader);
	this->queue = std::move(other.queue);
	return *this;
}

void HttpRequestQueue::push(const Response& item) {
	this->queue.push(item);
}

Response HttpRequestQueue::next() {
	if (!this->queue.size()) throw "empty queue";
	Response temp = this->queue.front();
	this->queue.pop();
	return temp;
}

bool HttpRequestQueue::await() {

	if (!reader.valid()) {
		return this->queue.size();
	}

	auto readerDone = false;
	while (!readerDone && !this->queue.size()) {
		readerDone = this->reader.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
	}

	if (readerDone) {
		this->reader.get();
	}

	return this->queue.size();
}

//typedef std::function<Response(const Request& request)> Callback;

/*HttpRequestQueue serve() {

	auto futurePtr = std::make_shared<HttpRequestQueue>();

	std::this_thread::sleep_for(std::chrono::milliseconds(5));

	futurePtr->reader = std::async([](std::shared_ptr<HttpRequestQueue> queue){
		for (size_t i = 0; i < 8; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			Response response;
			response.data = "response #" + std::to_string(i);
			queue->push(response);
		}
	}, futurePtr);

	std::this_thread::sleep_for(std::chrono::milliseconds(3));

	return futurePtr;
}*/

int main(int argc, char const *argv[]) {

	auto incoming = HttpRequestQueue();

	while (incoming.await()) {
		auto next = incoming.next();
		puts(next.data.c_str());
	}
	
	return 0;
}
