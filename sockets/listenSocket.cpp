#include "sockets.hpp"

HTTPSocket::ListenSocket::ListenSocket(const char* listenPort) {

	#ifdef _WIN32
	SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (temp == INVALID_SOCKET && GetLastError() == WSANOTINITIALISED) {
		WSADATA initdata;
		auto initStat = WSAStartup(MAKEWORD(2,2), &initdata);
		if (initStat) {
			socketStat = HSOCKERR_INIT;
			socketError = GetLastError();
			return;
		} else closesocket(temp);
	}
	#endif
	
	//	resolve server address
	struct addrinfo* servAddr = NULL;
	struct addrinfo addrHints;
	ZeroMemory(&addrHints, sizeof(addrHints));
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;
	addrHints.ai_protocol = IPPROTO_TCP;
	addrHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, listenPort, &addrHints, &servAddr) != 0) {

		#ifdef _WIN32
			socketError = GetLastError();
		#else
			socketError = errno;
		#endif
		socketStat = HSOCKERR_ADDRESS;

		freeaddrinfo(servAddr);
		return;
	}
	
	// create and bind a SOCKET
	this->hSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);

	if (this->hSocket == INVALID_SOCKET) {

		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_CREATE;

		freeaddrinfo(servAddr);
		return;
	}

	int reuseOptVal = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseOptVal, sizeof(reuseOptVal))) {
		
		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_SETOPT;

		freeaddrinfo(servAddr);
		closesocket(this->hSocket);
	}
	
	if (bind(this->hSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {

		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_BIND;

		freeaddrinfo(servAddr);
		closesocket(this->hSocket);

		return;
	}

	freeaddrinfo(servAddr);

	if (listen(this->hSocket, SOMAXCONN) == SOCKET_ERROR) {

		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_LISTEN;

		closesocket(this->hSocket);

		return;
	}
	socketStat = HSOCKERR_OK;
}

HTTPSocket::ListenSocket::~ListenSocket() {
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

HTTPSocket::ClientSocket HTTPSocket::ListenSocket::acceptConnection() {	
	return ClientSocket(this->hSocket);
}

bool HTTPSocket::ListenSocket::ok() {
	return socketStat == HSOCKERR_OK;
}
HTTPSocket::OpStatus HTTPSocket::ListenSocket::status() {
	return { socketStat, socketError };
}
