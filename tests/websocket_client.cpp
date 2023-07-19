#include "../server/server.hpp"
#include <iostream>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

int main(int argc, char const** argv) {

	auto websock = WebSocket(URL("ws://localhost:8080/"));

	websock.sendMessage("hello?");

	while (websock.isAlive()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (websock.availableMessage()) {

			auto messages = websock.getMessages();

			for (auto& msg : messages) {
				std::cout << msg.timestamp << ": " << msg.content << std::endl;

				if (msg.content == "go away") {
					websock.close(WebSocket::CloseCodes::normal);
					break;
				}
			}

			websock.sendMessage("got it!");
		}
	}

	std::cout << websock.getError().what() << std::endl;

	return 0;
}
