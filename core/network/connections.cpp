#include "./sysnetw.hpp"
#include "../network.hpp"

using namespace Network;

TCPListenSocket::TCPListenSocket(uint16_t listenPort) {

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->hSocket == INVALID_SOCKET) {
		auto apierror = getAPIError();
		throw std::runtime_error("failed to create listen socket: code " + std::to_string(apierror));
	}
	
	//	allow fast port reuse
	uint32_t sockoptReuseaddr = 1;
	if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&(sockoptReuseaddr), sizeof(sockoptReuseaddr))) {
		auto apierror = getAPIError();
		throw std::runtime_error("failed to set socket reuse address option: code " + std::to_string(apierror));
	}

	//	bind socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(listenPort);
	this->internalPort = listenPort;

	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		auto apierror = getAPIError();
		closesocket(this->hSocket);
		throw std::runtime_error("failed to bind socket: code " + std::to_string(apierror));
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN) == SOCKET_ERROR) {
		auto apierror = getAPIError();
		closesocket(this->hSocket);
		throw std::runtime_error("socket listen failed: code " + std::to_string(apierror));
	}
}

TCPListenSocket::~TCPListenSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

TCPConnection TCPListenSocket::acceptConnection() {

	ConnCreateInit next;
	next.info.port = this->internalPort;

	sockaddr_in peerAddr;
	socklen_t clientAddrLen = sizeof(peerAddr);

	next.hSocket = accept(this->hSocket, (sockaddr*)&peerAddr, &clientAddrLen);
	if (next.hSocket == INVALID_SOCKET) throw std::runtime_error("socket accept failed: code " + std::to_string(getAPIError()));

	char tempbuffIPv4[64];
	auto resolvedPeerIP = inet_ntop(AF_INET, &peerAddr.sin_addr, tempbuffIPv4, sizeof(tempbuffIPv4));
	if (resolvedPeerIP) next.info.ip = tempbuffIPv4;

	return TCPConnection(next);
}

bool TCPListenSocket::ok() {
	return this->hSocket != INVALID_SOCKET;
}


TCPConnection::TCPConnection(ConnCreateInit init) {

	this->conninfo = init.info;
	this->hSocket = init.hSocket;

	auto setConnectionTimeout = init.connTimeout ? init.connTimeout : static_cast<uint32_t>(Constants::Connection_TimeoutMs);

	if (setConnectionTimeout > static_cast<uint32_t>(Constants::Connection_TimeoutMs_Max))
		throw std::runtime_error("cannot create TCP connection: timeout is too big");
	if (setConnectionTimeout < static_cast<uint32_t>(Constants::Connection_TimeoutMs_Min))
		throw std::runtime_error("cannot create TCP connection: timeout is too small");

	setSocketTimeouts(this->hSocket, setConnectionTimeout);
}

TCPConnection::~TCPConnection() {
	this->closeConnection();
}

void TCPConnection::closeConnection() {

	if (this->hSocket == INVALID_SOCKET) return;

	if (shutdown(this->hSocket, SD_BOTH)) {
		throw std::runtime_error("failed to perform socket shutdown: code " + std::to_string(getAPIError()));
	}

	if (closesocket(this->hSocket)) {
		throw std::runtime_error("failed to close socket: code " + std::to_string(getAPIError()));
	}

	this->hSocket = INVALID_SOCKET;
}

TCPConnection& TCPConnection::operator= (TCPConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

const ConnInfo& TCPConnection::info() const noexcept {
	return this->conninfo;
}

bool TCPConnection::alive() {
	return this->hSocket != INVALID_SOCKET;
}

void TCPConnection::write(const std::vector<uint8_t>& data) {

	if (this->hSocket == INVALID_SOCKET)
		throw std::runtime_error("cann't write to a closed connection");
	auto sendResult = send(this->hSocket, (const char*)data.data(), data.size(), 0);

	if (static_cast<size_t>(sendResult) != data.size())
		throw std::runtime_error("network error while sending data: code " + std::to_string(getAPIError()));
}

std::vector<uint8_t> TCPConnection::read() {
	return this->read(static_cast<uint32_t>(Constants::Receive_ChunkSize));
}

std::vector<uint8_t> TCPConnection::read(size_t expectedSize) {

	if (this->hSocket == INVALID_SOCKET)
		throw std::runtime_error("can't read from a closed connection");

	std::vector<uint8_t> chunk;
	chunk.resize(expectedSize);

	auto bytesReceived = recv(this->hSocket, (char*)chunk.data(), chunk.size(), 0);
	if (bytesReceived == 0) {
		this->closeConnection();
		return {};
	} else if (bytesReceived < 0) {
		throw std::runtime_error("network error while getting data: code " + std::to_string(getAPIError()));
	}

	chunk.resize(bytesReceived);
	chunk.shrink_to_fit();

	return chunk;
}

void Network::setSocketTimeouts(SOCKET hSocket, uint32_t timeoutsMs) {
	if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket RX timeout: code " + std::to_string(getAPIError()));

	if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket TX timeout: code " + std::to_string(getAPIError()));
}
