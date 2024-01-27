#include "./listener.hpp"
#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

ListenSocket::ListenSocket(const ListenConfig& init) {

	//	check that connection timeout values are in acceptable range
	if (init.connectionTimeout > Connection::TimeoutMs_Max)
		throw std::runtime_error("Cannot create a TCP connection: timeout is too big");
	if (init.connectionTimeout < Connection::TimeoutMs_Min)
		throw std::runtime_error("Cannot create a TCP connection: timeout is too small");

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

	//	just save config for later
	this->config = init;

	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->hSocket == INVALID_SOCKET) {
		throw Lambda::APIError("failed to create listen socket");
	}
	
	//	allow fast port reuse
	if (init.allowPortReuse) {
		uint32_t sockoptReuseaddr = 1;
		if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
			auto apierror = Lambda::APIError("failed to create listen socket");
			closesocket(this->hSocket);
			throw apierror;
		}
		puts("Warning: fast port reuse enabled");
	}

	//	bind socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(init.port);

	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		auto apierror = Lambda::APIError("failed to bind socket");
		closesocket(this->hSocket);
		throw apierror;
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN)) {
		auto apierror = Lambda::APIError("socket listen failed");
		closesocket(this->hSocket);
		throw apierror;
	}
}

ListenSocket::~ListenSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

ListenSocket::ListenSocket(ListenSocket&& other) noexcept {
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
	next.info.timeout = this->config.connectionTimeout;
	next.info.remoteAddr.port = 80;
	next.info.remoteAddr.transport = ConnectionTransport::TCP;

	//	accept network connection
	sockaddr_in peerAddr;
	socklen_t clientAddrLen = sizeof(peerAddr);
	next.hSocket = accept(this->hSocket, (sockaddr*)&peerAddr, &clientAddrLen);

	//	verify that we have a valid socket
	if (next.hSocket == INVALID_SOCKET) {
		throw Lambda::APIError("socket accept failed");
	}

	//	try setting connection timeouts
	try {

		if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&next.info.timeout, sizeof(next.info.timeout)))
			throw Lambda::APIError("failed to set socket RX timeout");
		if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&next.info.timeout, sizeof(next.info.timeout)))
			throw Lambda::APIError("failed to set socket TX timeout");

	} catch(const std::exception& err) {
		if (next.hSocket != INVALID_SOCKET)
			closesocket(next.hSocket);
		throw err;
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
