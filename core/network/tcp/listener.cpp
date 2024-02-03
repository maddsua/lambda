#include "./listener.hpp"
#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

ListenSocket::ListenSocket(const ListenConfig& init) {

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

	//	just save config for later
	this->config = init;

	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->hSocket == Network::invalid_socket) {
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
	if (this->hSocket == Network::invalid_socket) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

ListenSocket::ListenSocket(ListenSocket&& other) noexcept {
	this->hSocket = other.hSocket;
	this->config = other.config;
	other.hSocket = Network::invalid_socket;
}

ListenSocket& ListenSocket::operator= (ListenSocket&& other) noexcept {
	this->hSocket = other.hSocket;
	this->config = other.config;
	other.hSocket = Network::invalid_socket;
	return *this;
}

std::optional<Connection> ListenSocket::acceptConnection() {

	//	check that we have a valid socket
	if (this->hSocket == Network::invalid_socket) {
		throw std::runtime_error("cannot accept anything from a closed socket");
	}

	//	accept network connection
	sockaddr_in peerAddr;
	socklen_t clientAddrLen = sizeof(peerAddr);
	auto nextSocket = accept(this->hSocket, (sockaddr*)&peerAddr, &clientAddrLen);

	//	verify that we have a valid socket
	if (nextSocket == Network::invalid_socket) {
		if (this->hSocket == Network::invalid_socket) return std::nullopt;
		throw Lambda::APIError("socket accept failed");
	}

	//	try getting peer host name
	char tempbuffIPv4[32];
	auto peerIpCString = inet_ntop(AF_INET, &peerAddr.sin_addr, tempbuffIPv4, sizeof(tempbuffIPv4));

	Address nextAddr {
		peerIpCString ? peerIpCString : "",
		80,
		ConnectionTransport::TCP
	};

	ConnectionInfo nextInfo {
		nextAddr,
		this->config.connectionTimeouts,
		this->config.port
	};

	auto nextconn = Connection({
		nextSocket,
		nextInfo
	});

	nextconn.setTimeouts(this->config.connectionTimeouts.receive, SetTimeoutsDirection::Receive);
	nextconn.setTimeouts(this->config.connectionTimeouts.send, SetTimeoutsDirection::Send);

	return nextconn;
}

bool ListenSocket::active() const noexcept {
	return this->hSocket != Network::invalid_socket;
}

const ListenConfig& ListenSocket::getConfig() const noexcept {
	return this->config;
}

void ListenSocket::stop() noexcept {

	if (this->hSocket == Network::invalid_socket) return;

	auto tempHandle = this->hSocket;
	this->hSocket = Network::invalid_socket;

	shutdown(tempHandle, SD_BOTH);
	closesocket(tempHandle);
}
