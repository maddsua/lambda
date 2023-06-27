#include "socket.hpp"
#include <chrono>

HTTPSocket::Server::Server() {

	auto socketInit = HTTPSocket::createAndListen(&ListenSocket, "8080");

	if (socketInit.code != HTTPSocket::HSOCKERR_OK) throw Lambda::exception("Failed to start server, code: " + std::to_string(socketInit.code));

	//requestCallback = lambdaCallbackFunction;
	running = true;
	handlerDispatched = true;
	watchdogThread = std::thread(connectionWatchdog, this);
}

HTTPSocket::Server::~Server() {
	running = false;
	if (watchdogThread.joinable())
		watchdogThread.join();
	disconnect(ListenSocket);	
}

void HTTPSocket::Server:: connectionWatchdog() {

	auto lastDispatched = std::chrono::system_clock::now();

	while (running) {

		if (!handlerDispatched) {

			if ((std::chrono::system_clock::now() - lastDispatched) > std::chrono::milliseconds(1)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				//puts(std::to_string((std::chrono::system_clock::now() - lastDispatched).count()).c_str());
			}

			continue;
		}

		lastDispatched = std::chrono::system_clock::now();
		auto invoked = std::thread(connectionHandler, this);
		handlerDispatched = false;
		invoked.detach();
	}
}

void HTTPSocket::Server::connectionHandler() {
	
	//	accept socket and free the flag for next handler instance
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	handlerDispatched = true;
	setConnectionTimeout(ClientSocket, 15000);

	auto request = receiveMessage(ClientSocket);

	puts(request.path().c_str());

	auto resp = HTTP::Response();

	resp.headers.fromEntries({{"Server", "maddsua/lambda"}});
	
	sendMessage(ClientSocket, resp);

	shutdown(ClientSocket, SD_BOTH);
	closesocket(ClientSocket);
}