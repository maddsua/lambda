#include <iostream>

#include "../core/polyfill.hpp"
#include "../core/network.hpp"

int main(int argc, char const *argv[]) {
	
	auto server = Network::TCPListenSocket(8180);

	std::cout << "Started server...\n";

	while (server.ok()) {

		auto conn = server.acceptConnection();
		std::cout << "Got a connection!\n-----\n";

		auto readData = conn.read();
		std::cout << std::string(readData.begin(), readData.end()) << "\n-----\n";

		std::string responseString = "HTTP/1.1 200 OK\r\nx-header: test value\r\n\r\n";

		conn.write(std::vector<uint8_t>(responseString.begin(), responseString.end()));
	}
	
	return 0;
}
