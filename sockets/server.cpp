#include "socket.hpp"

HTTPSocket::Server::Server() {

	handlerDispatched = false;

    SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (temp == INVALID_SOCKET){
		if (GetLastError() == WSANOTINITIALISED) {
			WSAStartup(MAKEWORD(2,2), nullptr);
		}
    }

    closesocket(temp);

	//	resolve server address
	struct addrinfo *servAddr = NULL;
	struct addrinfo addrHints;
		ZeroMemory(&addrHints, sizeof(addrHints));
		addrHints.ai_family = AF_INET;
		addrHints.ai_socktype = SOCK_STREAM;
		addrHints.ai_protocol = IPPROTO_TCP;
		addrHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, std::to_string(778).c_str(), &addrHints, &servAddr) != 0) {
		//if (!instanceConfig.mutlipeInstances) WSACleanup();
		freeaddrinfo(servAddr);
		return;
		/*return {
			false,
			"Localhost didn't resolve",
			"Code:" + std::to_string(GetLastError())
		};*/
	}

	// create and bind a SOCKET
	ListenSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(servAddr);
		//if (!instanceConfig.mutlipeInstances) WSACleanup();
		return;
		/*return {
			false,
			"Failed to create listening socket",
			"Code:" + std::to_string(GetLastError())
		};*/
	}

	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		//if (!instanceConfig.mutlipeInstances) WSACleanup();
		return;
		/*return {
			false,
			"Failed to bind a TCP socket",
			"Code:" + std::to_string(GetLastError())
		};*/
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		//if (!instanceConfig.mutlipeInstances) WSACleanup();
		return;
		/*return {
			false,
			"Socket error",
			"Code:" + std::to_string(GetLastError())
		};*/
	}
}

HTTPSocket::Server::~Server() {

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

	while (true) {
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

	//	set socket timeouts
	/*const uint32_t tcpTimeout = LAMBDA_TCP_TIMEOUT_MS;
	setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tcpTimeout, sizeof(uint32_t));
	setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tcpTimeout, sizeof(uint32_t));*/

	auto request = receiveMessage(&ClientSocket);

	puts(request.path().c_str());

	auto resp = HTTP::Response();

	resp.headers = {{"test-header-5", "ok"}};
	

	sendMessage(&ClientSocket, resp);

	closesocket(ClientSocket);
}