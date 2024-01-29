#include <iostream>

#include "../lambda.hpp"
#include "../core/network/tcp/listener.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	auto listener = Network::TCP::ListenSocket();

	std::cout << "Started server at http://localhost:" + std::to_string(listener.getConfig().port) + "/\n";

	while (listener.active()) {

		auto conn = listener.acceptConnection().value();
		std::cout << "Got a connection!\n-----\n";

		auto readData = conn.read();
		std::cout << std::string(readData.begin(), readData.end()) << "\n-----\n";

		std::string responseString = "HTTP/1.1 200 OK\r\nx-header: test value\r\n\r\n";

		conn.write(std::vector<uint8_t>(responseString.begin(), responseString.end()));
	}

	return 0;
}
