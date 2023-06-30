#include "../server/server.hpp"
#include <iostream>

using namespace Lambda;

HTTP::Response callbackServerless(HTTP::Request& request, Lambda::Context& context) {
	std::cout << "Request to \"" << request.path << "\" from " << request.headers.get("user-agent") << std::endl;
	return HTTP::Response({{"x-serverless", "true"}}, "success! your user-agent is: " + request.headers.get("user-agent"));
};

void callback(Lambda::Network::HTTPConnection& connection, Lambda::Context& context) {

	auto request = connection.receiveMessage();

	std::cout << "request headers: " << request.headers.stringify() << std::endl;

	auto websock = connection.upgradeToWebsocket(request);

	auto sendresult = websock.sendMessage("test message");

	if (sendresult.isError()) {
		std::cout << sendresult.what() << std::endl;
	}

	websock.sendMessage("lets send a very long test message thats is gonna be bigger that 127 bytes so websock actually sends it as 16 bit integer and we'll se how it works in reality");

	while (websock.isAlive()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (websock.availableMessage()) {

			auto messages = websock.getMessages();

			for (auto& msg : messages) {
				std::cout << msg.timestamp << ": " << msg.message << std::endl;
			}

			websock.sendMessage("got it!");
		}

		//websock.close(Lambda::Network::WSCLOSE_GOING_AWAY);
	}

	std::cout << "Websocket disconnected: " << websock.getError().what() << std::endl;

}

int main() {

	auto server = Lambda::Server();

	//server.setServerlessCallback(&callbackServerless);
	server.setServerCallback(&callback);

	while (true) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return 0;
}
