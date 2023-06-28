#include "../server/server.hpp"
#include <windows.h>

using namespace Lambda;

HTTP::Response callback(HTTP::Request& request, Lambda::Context& context) {
	puts(request.path().c_str());
	return HTTP::Response({{"x-serverless", "true"}}, "success! your user-agent is: " + request.headers().get("user-agent"));
};

int main() {

	auto server = Lambda::Server();

	server.setServerlessCallback(&callback);

	while (true) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		Sleep(10);
	}
	
	return 0;
}
