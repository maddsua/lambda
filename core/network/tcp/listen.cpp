#include "./listen.hpp"
#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

ListenSocket::ListenSocket(const ListenConfig& init) {

	if (init.timeouts && init.timeouts > Connection::TimeoutMs_Max)
		throw std::runtime_error("Cannot create a TCP connection: timeout is too big");

	//	special treatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

	//	just save config for later
	this->config = init;

	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->hSocket == INVALID_SOCKET) {
		throw std::runtime_error("failed to create listen socket: code " + std::to_string(getAPIError()));
	}
	
	//	allow fast port reuse
	if (init.allowPortReuse) {
		uint32_t sockoptReuseaddr = 1;
		if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
			auto apierror = getAPIError();
			closesocket(this->hSocket);
			throw std::runtime_error("failed to set socket reuse address option: code " + std::to_string(apierror));
		}
		puts("Warning: fast port reuse enabled");
	}

	//	bind socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(init.port);

	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		auto apierror = getAPIError();
		closesocket(this->hSocket);
		if (apierror == LNE_ADDRINUSE) throw std::runtime_error("failed to bind socket: address already in use");
		throw std::runtime_error("failed to bind socket: code " + std::to_string(apierror));
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN)) {
		auto apierror = getAPIError();
		closesocket(this->hSocket);
		throw std::runtime_error("socket listen failed: code " + std::to_string(apierror));
	}
}

ListenSocket::~ListenSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

ListenSocket::ListenSocket(ListenSocket&& other) {
	this->hSocket = other.hSocket;
	this->config = other.config;
	other.hSocket = INVALID_SOCKET;
}

ListenSocket& ListenSocket::operator= (ListenSocket&& other) noexcept {
	this->hSocket = other.hSocket;
	this->config = other.config;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

Connection ListenSocket::acceptConnection() {

	//	check that we have a valid socket
	if (this->hSocket == INVALID_SOCKET) {
		throw std::runtime_error("cannot accept anything from a closed socket");
	}

	//	copy connection params
	ConnCreateInit next;
	next.info.hostPort = this->config.port;
	next.info.remoteAddr.port = 80;
	next.info.remoteAddr.transport = ConnectionTransport::TCP;
	next.info.timeouts.rx = this->config.timeouts;
	next.info.timeouts.tx = this->config.timeouts;
	
	//	accept network connection
	sockaddr_in peerAddr;
	socklen_t clientAddrLen = sizeof(peerAddr);
	next.hSocket = accept(this->hSocket, (sockaddr*)&peerAddr, &clientAddrLen);

	//	verify that we have a valid socket
	if (next.hSocket == INVALID_SOCKET) {
		throw std::runtime_error("socket accept failed: code " + std::to_string(getAPIError()));
	}

	//	try getting peer host name
	char tempbuffIPv4[64];
	if (inet_ntop(AF_INET, &peerAddr.sin_addr, tempbuffIPv4, sizeof(tempbuffIPv4))) {
		next.info.remoteAddr.hostname = tempbuffIPv4;
	}

	return Connection(next);
}

bool ListenSocket::ok() const noexcept {
	return this->hSocket != INVALID_SOCKET;
}

const ListenConfig& ListenSocket::getConfig() const noexcept {
	return this->config;
}
