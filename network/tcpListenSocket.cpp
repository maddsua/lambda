#include "./network.hpp"
#include "./tcpip.hpp"

using namespace Lambda;
using namespace Lambda::Network;

ListenSocket::ListenSocket(uint16_t listenPort) {

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

	//	server address
	sockaddr_in serverInfo;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverInfo.sin_port = htons(listenPort);
	
	// create and bind a SOCKET
	this->hSocket = socket(serverInfo.sin_family, SOCK_STREAM, IPPROTO_TCP);
	if (this->hSocket == INVALID_SOCKET) {
		auto errcode = getAPIError();
		throw Lambda::Error("Could not resolve localhost", errcode);
	}

	//	allow fast port reuse
	uint32_t sockoptReuseaddr = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
		auto errcode = getAPIError();
		throw Lambda::Error("Failed to set socket reuse address option", errcode);
	}
	
	//	bind socket
	if (bind(this->hSocket, (sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR) {
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
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

bool ListenSocket::isAlive() {
	return this->hSocket != INVALID_SOCKET;
}

HTTPServer ListenSocket::acceptConnection() {
	return HTTPServer(this->hSocket);
}
