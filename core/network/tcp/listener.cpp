#include "./listener.hpp"
#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

ListenSocket::ListenSocket(const ListenConfig& init) : config(init) {

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->hSocket == INVALID_SOCKET) {
		throw NetworkError("Failed to create listen socket", Lambda::OS_Error());
	}

	//	allow fast port reuse
	if (init.allowPortReuse) {
		uint32_t sockoptReuseaddr = 1;
		if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
			auto apierror = NetworkError("Failed to enable fast port reuse", Lambda::OS_Error());
			closesocket(this->hSocket);
			throw apierror;
		}
	}

	//	bind socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(init.port);

	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		auto apierror = NetworkError("Failed to bind() socket", Lambda::OS_Error());
		closesocket(this->hSocket);
		throw apierror;
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN)) {
		auto apierror = NetworkError("Socket listen() failed", Lambda::OS_Error());
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

std::optional<Connection> ListenSocket::acceptConnection() {

	//	check that we have a valid socket
	if (this->hSocket == INVALID_SOCKET) {
		throw NetworkError("Cannot accept anything from a closed socket");
	}

	//	accept network connection
	sockaddr_in peerAddr;
	socklen_t clientAddrLen = sizeof(peerAddr);
	auto nextSocket = accept(this->hSocket, (sockaddr*)&peerAddr, &clientAddrLen);

	//	verify that we have a valid socket
	if (nextSocket == INVALID_SOCKET) {
		if (this->hSocket == INVALID_SOCKET) return std::nullopt;
		throw NetworkError("Socket accept() failed", Lambda::OS_Error());
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
	return this->hSocket != INVALID_SOCKET;
}

const ListenConfig& ListenSocket::getConfig() const noexcept {
	return this->config;
}

void ListenSocket::stop() noexcept {

	if (this->hSocket == INVALID_SOCKET) return;

	auto tempHandle = this->hSocket;
	this->hSocket = INVALID_SOCKET;

	shutdown(tempHandle, SD_BOTH);
	closesocket(tempHandle);
}
