#include <cstdio>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <memory>

class HttpRequestQueue {
	private:
		std::future<void> reader;
		std::queue<Response> queue;
	private:
		bool await();
		Response next();
};

bool HttpRequestQueue::await() {
	
}

struct Response {
	std::string data;
};

struct Request {
	std::string data;
};

//typedef std::function<Response(const Request& request)> Callback;

std::shared_ptr<HttpRequestQueue> serve() {

	auto futurePtr = std::make_shared<HttpRequestQueue>();

	//std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::thread([](std::shared_ptr<HttpRequestQueue> queue) {
		for (size_t i = 0; i < 8; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			Response response;
			response.data = "response #" + std::to_string(i);
			queue->push(response);
		}
	}, futurePtr).detach();

	return futurePtr;
}

int main(int argc, char const *argv[]) {

	auto incoming = serve();

	while (true) {
		if (!incoming->size()) continue;
		puts(incoming->front().data.c_str());
		incoming->pop();
	}
	
	return 0;
}
