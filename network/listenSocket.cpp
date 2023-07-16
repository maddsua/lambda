#include "./network.hpp"
#include "./tcpip.hpp"

using namespace Lambda;
using namespace Lambda::Network;

void ListenSocket::create(uint16_t listenPort, ConnectionProtocol proto) {

	//	server address
	struct {
		int family = AF_INET;
		int type = 0;
		int protocol = 0;
	} serverInfo;

	switch (proto) {

		case ConnectionProtocol::UDP: {
			serverInfo.type = SOCK_DGRAM;
			serverInfo.protocol = IPPROTO_UDP;
		} break;
	
		default: {
			serverInfo.type = SOCK_STREAM;
			serverInfo.protocol = IPPROTO_TCP;
		} break;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = serverInfo.family;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(listenPort);
	
	// create and bind a socket
	#ifdef _WIN32
	bool wsaInitEcexuted = false;
	#endif

	sockcreate:
	this->hSocket = socket(serverInfo.family, serverInfo.type, serverInfo.protocol);

	if (this->hSocket == INVALID_SOCKET) {

		auto apierror = getAPIError();

		#ifdef _WIN32
		if (apierror == WSANOTINITIALISED && !wsaInitEcexuted) {

			wsaInitEcexuted = true;
			WSADATA initdata;
			if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
				throw Lambda::Error("WSA initialization failed", apierror);
			goto sockcreate;

		} else
		#endif
		
		throw Lambda::Error("Failed to create listen socket", apierror);
	}

	//	allow fast port reuse
	uint32_t sockoptReuseaddr = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
		auto errcode = getAPIError();
		throw Lambda::Error("Failed to set socket reuse address option", errcode);
	}
	
	//	bind socket
	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		auto errcode = getAPIError();
		closesocket(this->hSocket);
		throw Lambda::Error("Failed to bind socket", errcode);
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN) == SOCKET_ERROR) {
		auto errcode = getAPIError();
		closesocket(this->hSocket);
		throw Lambda::Error("Socket listen failed", errcode);
	}
}

ListenSocket::~ListenSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

bool ListenSocket::isAlive() {
	return this->hSocket != INVALID_SOCKET;
}


TCPListenSocket::TCPListenSocket(uint16_t listenPort) {
	try {
		this->create(listenPort, ConnectionProtocol::TCP);
	} catch(const std::exception& e) {
		throw Lambda::Error("Could not create listen socket", e);
	}
}

HTTPConnection TCPListenSocket::acceptConnection() {
	return HTTPConnection(this->hSocket);
}
