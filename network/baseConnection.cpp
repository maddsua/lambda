#include <algorithm>
#include <cstring>
#include "./network.hpp"
#include "./tcpip.hpp"
#include "../compress/compress.hpp"

using namespace Lambda::HTTP;
using namespace Lambda::Network;
using namespace Lambda::Compress;

void BaseConnection::resolveAndConnect(const char* host, const char* port, ConnectionProtocol proto) {

	struct addrinfo* resolvedAddresses = nullptr;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	switch (proto) {

		case ConnectionProtocol::UDP: {
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;
		} break;
	
		default: {
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
		} break;
	}
	
	//	resolve host
	//	failsafe in case WSA gets glitchy
	#ifdef _WIN32
	bool wsaInitEcexuted = false;
	#endif

	dnsresolvehost:	//	yes, I'm using jumps here. deal with it.
	if (getaddrinfo(host, port, &hints, &resolvedAddresses) != 0) {

		auto apierror = getAPIError();

		#ifdef _WIN32
		if (apierror == WSANOTINITIALISED && !wsaInitEcexuted) {

			wsaInitEcexuted = true;
			WSADATA initdata;
			if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
				throw Lambda::Error("WSA initialization failed", apierror);
			goto dnsresolvehost;

		} else	//	this "else" goes to the "return" right below, don't panic.
		//	It looks like ass, but I fell more comfortable doing this,
		//	than bringing Boost ASIO or some other libarary to have crossplatform sockets
		#endif

		throw Lambda::Error("Failed to resolve host", apierror);
	}

	this->hSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (this->hSocket == INVALID_SOCKET) {
		auto apierror = getAPIError();
		freeaddrinfo(resolvedAddresses);
		throw Lambda::Error("Failed to create a socket", apierror);
	}

	size_t connectAttempts = 0;
	int64_t lastError = 0;

    for (auto addrPtr = resolvedAddresses; addrPtr != nullptr; addrPtr = addrPtr->ai_next) {

		if (connectAttempts > network_dnsresolve_attempts) {
			auto apierror = getAPIError();
			freeaddrinfo(resolvedAddresses);
			throw Lambda::Error("Reached max attempts (" + std::to_string(network_dnsresolve_attempts)  +") without succeding", apierror);
		}

		//	return socket if it's good
		if (connect(this->hSocket, addrPtr->ai_addr, addrPtr->ai_addrlen) != SOCKET_ERROR) {
			freeaddrinfo(resolvedAddresses);
			return;
		}

		lastError = getAPIError();
    }

	//	could not connect to any host;
	//	cleanup resolved addresses
	freeaddrinfo(resolvedAddresses);

	//	and kill socket if is open
	if (this->hSocket != INVALID_SOCKET)
		closesocket(this->hSocket);

	throw Lambda::Error("Could not connect to the server", lastError);
}

void BaseConnection::connectLocalSerivce(uint16_t servicePort, ConnectionProtocol proto) {

	struct {
		int family = AF_INET;
		int type = 0;
		int protocol = 0;
	} lhInfo;

	switch (proto) {

		case ConnectionProtocol::UDP: {
			lhInfo.type = SOCK_DGRAM;
			lhInfo.protocol = IPPROTO_UDP;
		} break;
	
		default: {
			lhInfo.type = SOCK_STREAM;
			lhInfo.protocol = IPPROTO_TCP;
		} break;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = lhInfo.family;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(servicePort);
	
	// create and bind a socket
	#ifdef _WIN32
	bool wsaInitEcexuted = false;
	#endif

	sockcreate:
	this->hSocket = socket(lhInfo.family, lhInfo.type, lhInfo.protocol);

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
 
	auto connectResult = connect(this->hSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (connectResult == SOCKET_ERROR) throw Lambda::Error("Socket failed to connect", getAPIError());
}

BaseConnection::~BaseConnection() {
	if (this->hSocket != INVALID_SOCKET) {
		shutdown(this->hSocket, SD_BOTH);
		closesocket(this->hSocket);
	}
}

void BaseConnection::setTimeouts(uint32_t timeoutMs) {

	if (timeoutMs == 0 || timeoutMs == -1) timeoutMs = network_connection_timeout;

	if (setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs)) != 0)
		throw Lambda::Error("Failed to set socket RX timeout", getAPIError());

	if (setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs)) != 0)
		throw Lambda::Error("Failed to set socket TX timeout", getAPIError());
}

SOCKET BaseConnection::getHandle() noexcept {
	return this->hSocket;
}

BaseConnection& BaseConnection::operator= (BaseConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

const std::string& BaseConnection::getPeerIPv4() noexcept {
	return this->peerIPv4;
}
