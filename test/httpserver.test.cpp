#include <iostream>
#include <future>
#include <chrono>
#include <thread>

#include "../core/polyfill.hpp"
#include "../core/http.hpp"
#include "../core/server.hpp"

using namespace Lambda;

HTTP::Response httpHandler(HTTP::Request req, Network::ConnInfo info) {
	printf("Serving rq for: %s\n", req.url.pathname.c_str());
	if (req.body.size()) printf("Request payload: %s\n", req.body.text().c_str());
	return HTTP::Response("status report: live");
}

int main(int argc, char const *argv[]) {

	auto startAtPort = 8180;

	auto server = HttpServer(httpHandler, {});

	std::cout << "Started server at http://localhost:" + std::to_string(startAtPort) + "/\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	auto emptyPromise = std::promise<void>();
	emptyPromise.get_future().wait();

	return 0;
}
