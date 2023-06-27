#include "socket.hpp"

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
	time_t lastDispatched = 0;

	/*while (running) {
		if(handlerDispatched) {
			auto invoked = std::thread(handler, this);
			handlerDispatched = false;
			lastDispatched = timeGetTime();
			invoked.detach();
			
		} else if (timeGetTime() > (lastDispatched + LAMBDA_DSP_SLEEP)) Sleep(LAMBDA_DSP_SLEEP);
	}*/

	while (running) {

		if (!handlerDispatched) {
			Sleep(1);
			continue;
		}

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