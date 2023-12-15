#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

HTTP::Response httpHandler(const Request& req, const Context& context)  {
	context.console.log({"Serving rq for:", req.url.pathname});
	if (req.body.size()) printf("Request payload: %s\n", req.body.text().c_str());
	return HTTP::Response("status report: live");
}

int main(int argc, char const *argv[]) {

	auto startAtPort = 8180;

	auto server = Network::TCPListenSocket(startAtPort, {});

	std::cout << "Started server at http://localhost:" + std::to_string(startAtPort) + "/\n";

	while (server.ok()) {

		auto conn = server.acceptConnection();
		std::cout << "Got a connection!\n-----\n";

		try {
			Server::handleHTTPConnection(std::move(conn), httpHandler, {});
			std::cout << "TCP connection served and closed\n-----\n";
		} catch(const std::exception& e) {
			std::cerr << "http handler crashed: " << e.what() << '\n';
		}
	}

	return 0;
}
