#include <stdexcept>
#include <cstdio>

#include "./sysnetw.hpp"
#include "./internal.hpp"
#include "../network.hpp"

using namespace Lambda;
using namespace Lambda::Network;

TCPListenSocket::TCPListenSocket(uint16_t listenPort, const ListenInit& init) {

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		wsaWakeUp();
	#endif

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
	serverAddr.sin_port = htons(listenPort);
	this->internalPort = listenPort;

	if (bind(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		auto apierror = getAPIError();
		closesocket(this->hSocket);
		if (apierror == LNETWERR_IN_USE) throw std::runtime_error("failed to bind socket: address already in use");
		throw std::runtime_error("failed to bind socket: code " + std::to_string(apierror));
	}

	//	listen for incoming connections
	if (listen(this->hSocket, SOMAXCONN)) {
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

TCPListenSocket::TCPListenSocket(TCPListenSocket&& other) {
	this->hSocket = other.hSocket;
	this->internalPort = other.internalPort;
	other.hSocket = INVALID_SOCKET;
}

TCPListenSocket& TCPListenSocket::operator= (TCPListenSocket&& other) noexcept {
	this->hSocket = other.hSocket;
	this->internalPort = other.internalPort;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

TCPConnection TCPListenSocket::acceptConnection() {

	if (this->hSocket == INVALID_SOCKET) throw std::runtime_error("cannot accept anything from a closed socket");

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

bool TCPListenSocket::ok() const noexcept {
	return this->hSocket != INVALID_SOCKET;
}

uint16_t TCPListenSocket::getPort() const noexcept {
	return this->internalPort;
}


TCPConnection::TCPConnection(ConnCreateInit init) {

	this->conninfo = init.info;
	this->hSocket = init.hSocket;

	auto setConnectionTimeout = init.connTimeout ? init.connTimeout : static_cast<uint32_t>(Constants::Connection_TimeoutMs);

	try {

		if (setConnectionTimeout > static_cast<uint32_t>(Constants::Connection_TimeoutMs_Max))
			throw std::runtime_error("cannot setup a TCP connection: timeout is too big");
			
		if (setConnectionTimeout < static_cast<uint32_t>(Constants::Connection_TimeoutMs_Min))
			throw std::runtime_error("cannot setup a TCP connection: timeout is too small");

		setConnectionTimeouts(this->hSocket, setConnectionTimeout);

	} catch(...) {
		if (this->hSocket == INVALID_SOCKET) return;
		closesocket(this->hSocket);
	}
}

TCPConnection& TCPConnection::operator= (TCPConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->conninfo = other.conninfo;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

TCPConnection::TCPConnection(TCPConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->conninfo = other.conninfo;
	other.hSocket = INVALID_SOCKET;
}

TCPConnection::~TCPConnection() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

void TCPConnection::close() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
	this->hSocket = INVALID_SOCKET;
}

const ConnInfo& TCPConnection::info() const noexcept {
	return this->conninfo;
}

bool TCPConnection::alive() const noexcept {
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

		this->close();
		return {};

	} else if (bytesReceived < 0) {

		auto apiError = getAPIError();

		switch (apiError) {

			case LNETWERR_TIMED_OUT: {
				this->close();
				return {};
			} break;
			
			default:
				throw std::runtime_error("network error while getting data: code " + std::to_string(apiError));
		}		
	}

	chunk.resize(bytesReceived);
	chunk.shrink_to_fit();

	return chunk;
}

void Network::setConnectionTimeouts(SOCKET hSocket, uint32_t timeoutsMs) {
	if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket RX timeout: code " + std::to_string(getAPIError()));
	if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket TX timeout: code " + std::to_string(getAPIError()));
}
