#include "./socketsapi.hpp"
#include "./sockets.hpp"

using namespace Lambda;

Socket::HTTPListenSocket::HTTPListenSocket(const char* listenPort) {

	//	initialize winsock2, windows only, obviously
	#ifdef _WIN32
	SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (temp == INVALID_SOCKET && GetLastError() == WSANOTINITIALISED) {

		WSADATA initdata;
		auto initStat = WSAStartup(MAKEWORD(2,2), &initdata);

		if (initStat != 0) {

			#ifdef _WIN32
				this->_status.apierror = GetLastError();
			#else 
				this->_status.apierror = errno;
			#endif
			this->_status.code = LAMBDASOCK_INIT;
			
			return;
			
		} else closesocket(temp);
	}
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

		#ifdef _WIN32
			this->_status.apierror = GetLastError();
		#else 
			this->_status.apierror = errno;
		#endif
		this->_status.code = LAMBDASOCK_ADDRESS;

		freeaddrinfo(servAddr);
		return;
	}
	
	// create and bind a SOCKET
	this->hSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (this->hSocket == INVALID_SOCKET) {

		#ifdef _WIN32
			this->_status.apierror = GetLastError();
		#else 
			this->_status.apierror = errno;
		#endif
		this->_status.code = LAMBDASOCK_CREATE;

		freeaddrinfo(servAddr);
		return;
	}

	//	allow fast port reuse
	uint32_t sockoptReuseaddr = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
		
		#ifdef _WIN32
			this->_status.apierror = GetLastError();
		#else 
			this->_status.apierror = errno;
		#endif
		this->_status.code = LAMBDASOCK_SETOPT;

		freeaddrinfo(servAddr);
		closesocket(this->hSocket);
	}
	
	//	bind socket
	if (bind(this->hSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {

		#ifdef _WIN32
			this->_status.apierror = GetLastError();
		#else 
			this->_status.apierror = errno;
		#endif
		this->_status.code = LAMBDASOCK_BIND;

		freeaddrinfo(servAddr);
		closesocket(this->hSocket);

		return;
	}

	freeaddrinfo(servAddr);

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN) == SOCKET_ERROR) {

		#ifdef _WIN32
			this->_status.apierror = GetLastError();
		#else 
			this->_status.apierror = errno;
		#endif
		this->_status.code = LAMBDASOCK_LISTEN;

		closesocket(this->hSocket);

		return;
	}
	
	this->_status.code = LAMBDASOCK_OK;
}

Socket::HTTPListenSocket::~HTTPListenSocket() {
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}
