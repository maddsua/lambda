#include <iostream>

#include "../core/polyfill.hpp"
#include "../core/http.hpp"
#include "../core/server.hpp"

HTTP::Response httpHandler(HTTP::Request req, Network::ConnInfo info) {
	printf("Serving rq for: %s\n", req.url.pathname.c_str());
	if (req.body.size()) printf("Request payload: %s\n", req.body.text().c_str());
	return HTTP::Response("status report: live");
}

int main(int argc, char const *argv[]) {
	
	auto server = Network::TCPListenSocket(8180);

	std::cout << "Started server...\n";

	while (server.ok()) {

		auto conn = server.acceptConnection();
		std::cout << "Got a connection!\n-----\n";

		try {
			Lambda::handleHTTPConnection(conn, httpHandler);
			std::cout << "TCP connection served and closed\n-----\n";
		} catch(const std::exception& e) {
			std::cerr << "http handler crashed: " << e.what() << '\n';
		}
	}

	return 0;
}
