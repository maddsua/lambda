#include <cstdio>
#include <thread>

#include "./net.hpp"
#include "../utils/tests.hpp"

void simple_handler(Lambda::Net::TcpConnection&& conn) {

	auto request = conn.read(16384);

	fprintf(stderr, "Request from %s: \n%s----\n", conn.remote_addr().hostname.c_str(), request.data());

	std::string response = "http/1.1 200 OK\r\n\r\n";

	conn.write(std::vector<uint8_t>(response.begin(), response.end()));
}

int main() {

	auto listener = Lambda::Net::TcpListener({
		.port = 8001
	});

	listener.bind_and_listen();

	fprintf(stderr, "Listening at: http://localhost:%i/\n", static_cast<int>(listener.options.port));

	while (listener.is_listening()) {

		try {
			auto next = listener.next();
			std::thread(simple_handler, std::move(next)).detach();
		} catch(const std::exception& e) {
			printf("Caught an exception: %s\n", e.what());
		}
	}

	return 0;
}
