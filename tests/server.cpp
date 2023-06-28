#include "../server/server.hpp"
#include <iostream>

using namespace Lambda;

HTTP::Response callback(HTTP::Request& request, Lambda::Context& context) {
	std::cout << "Request to \"" << request.path << "\" from " << request.headers.get("user-agent") << std::endl;
	return HTTP::Response({{"x-serverless", "true"}}, "success! your user-agent is: " + request.headers.get("user-agent"));
};

int main() {

	auto server = Lambda::Server();

	server.setServerlessCallback(&callback);

	while (true) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return 0;
}
