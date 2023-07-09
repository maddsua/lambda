#include "./network.hpp"
#include "./tcpip.hpp"

using namespace Lambda;
using namespace Lambda::Network;

ListenSocket::ListenSocket(const char* listenPort) {

	//	initialize winsock2, windows only, obviously
	#ifdef _WIN32
	SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (temp == INVALID_SOCKET) {

		if (GetLastError() == WSANOTINITIALISED) {

			WSADATA initdata;
			if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
				throw Lambda::Error("WSA initialization failed", getAPIError());
		}
		
	} else closesocket(temp);
	#endif
	
	//	some network hints
	struct addrinfo* servAddr = NULL;
	struct addrinfo addrHints;
	memset(&addrHints, 0, sizeof(addrHints));
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;
	addrHints.ai_protocol = IPPROTO_TCP;
	addrHints.ai_flags = AI_PASSIVE;

	//	resolve server address
	if (getaddrinfo(NULL, listenPort, &addrHints, &servAddr) != 0) {
		auto errcode = getAPIError();
		freeaddrinfo(servAddr);
		throw Lambda::Error("Could not resolve localhost", errcode);
	}
	
	// create and bind a SOCKET
	this->hSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (this->hSocket == INVALID_SOCKET) {
		auto errcode = getAPIError();
		freeaddrinfo(servAddr);
		throw Lambda::Error("Could not resolve localhost", errcode);
	}

	//	allow fast port reuse
	uint32_t sockoptReuseaddr = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
		auto errcode = getAPIError();
		freeaddrinfo(servAddr);
		throw Lambda::Error("Failed to set socket reuse address option", errcode);
	}
	
	//	bind socket
	if (bind(this->hSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		auto errcode = getAPIError();
		freeaddrinfo(servAddr);
		closesocket(this->hSocket);
		throw Lambda::Error("Failed to bind socket", errcode);
	}

	freeaddrinfo(servAddr);

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN) == SOCKET_ERROR) {
		auto errcode = getAPIError();
		closesocket(this->hSocket);
		throw Lambda::Error("Listen command failed", errcode);
	}	
}

ListenSocket::~ListenSocket() {
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

bool ListenSocket::isAlive() {
	return this->hSocket != INVALID_SOCKET;
}

HTTPServer ListenSocket::acceptConnection() {
	return HTTPServer(this->hSocket);
}
