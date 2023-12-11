#include <iostream>
#include <stdexcept>
#include <future>
#include <queue>
#include <chrono>
#include <mutex>
#include <thread>

std::string processRequest(const std::string& input, size_t id) {
	if (id == 4) throw std::runtime_error("test exception");
	return input + ' ' + std::to_string(id);
};

int main(int argc, char const *argv[]) {

	std::queue<std::future<std::string>> pipeline;
	std::mutex pipelineMtLock;

	auto receiveInited = std::promise<void>();
	auto receiveDone = std::promise<void>();

	auto receiveLoop = std::async([&]() {

		auto data = std::string("Request");

		std::lock_guard<std::mutex>lock(pipelineMtLock);

		pipeline.push(std::async(processRequest, data, 1));
		receiveInited.set_value();

		std::this_thread::sleep_for(std::chrono::milliseconds(25));

		pipeline.push(std::async(processRequest, data, 2));

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		pipeline.push(std::async(processRequest, data, 3));
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		pipeline.push(std::async(processRequest, data, 4));
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		pipeline.push(std::async(processRequest, data, 5));

		receiveDone.set_value();

	});

	receiveInited.get_future().wait();
	auto receiveDoneFuture = receiveDone.get_future();

	while (receiveDoneFuture.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready || pipeline.size()) {

		if (!pipeline.size()) continue;
		else if (pipeline.front().wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) continue;

		try {
			std::cout << "Success: " << pipeline.front().get() << "\n";
		} catch(const std::exception& e) {
			std::cerr << "Failed: " << e.what() << '\n';
		}
		
		std::lock_guard<std::mutex>lock(pipelineMtLock);
		pipeline.pop();
	}

	return 0;
}
