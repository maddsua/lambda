#include "socket.hpp"

HTTPSocket::Server::Server() {

	handlerDispatched = false;

	#ifdef _WIN32

		SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (temp == INVALID_SOCKET) {
			if (GetLastError() == WSANOTINITIALISED) {
				WSADATA initdata;
				auto initStat = WSAStartup(MAKEWORD(2,2), &initdata);
				if (initStat) throw Lambda::exception("WSAStartup failed, code: " + std::to_string(GetLastError()));
			}
		} else closesocket(temp);

	#endif

	//	resolve server address
	struct addrinfo *servAddr = NULL;
	struct addrinfo addrHints;
		ZeroMemory(&addrHints, sizeof(addrHints));
		addrHints.ai_family = AF_INET;
		addrHints.ai_socktype = SOCK_STREAM;
		addrHints.ai_protocol = IPPROTO_TCP;
		addrHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, std::to_string(778).c_str(), &addrHints, &servAddr) != 0) {
		freeaddrinfo(servAddr);
		throw Lambda::exception("Localhost didn't resolve, code: " + std::to_string(GetLastError()));
	}

	// create and bind a SOCKET
	ListenSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(servAddr);
		throw Lambda::exception("Failed to create listening socket, code: " + std::to_string(GetLastError()));
	}

	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		throw Lambda::exception("Failed to bind a TCP socket, code: " + std::to_string(GetLastError()));
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		throw Lambda::exception("Socket listen error, code: " + std::to_string(GetLastError()));
	}

	//requestCallback = lambdaCallbackFunction;
	running = true;
	watchdogThread = std::thread(connectionWatchdog, this);
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