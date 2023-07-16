#include <algorithm>
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

	SOCKET clientSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (clientSocket == INVALID_SOCKET) {
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
		if (connect(clientSocket, addrPtr->ai_addr, addrPtr->ai_addrlen) != SOCKET_ERROR) {
			freeaddrinfo(resolvedAddresses);
			return;
		}

		lastError = getAPIError();
    }

	//	cleanup resolved addresses
	freeaddrinfo(resolvedAddresses);

	//	kill socket if open
	if (clientSocket != INVALID_SOCKET)
		closesocket(clientSocket);

	throw Lambda::Error("Could not connect to the server", lastError);
}

BaseConnection::~BaseConnection() {
	if (this->hSocket != INVALID_SOCKET) {
		shutdown(this->hSocket, SD_BOTH);
		closesocket(this->hSocket);
	}
}

Lambda::Error BaseConnection::setTimeouts(uint32_t timeoutMs) {

	if (timeoutMs == 0 || timeoutMs == -1) timeoutMs = network_connection_timeout;

	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	auto setOptStatTX = setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	if (setOptStatRX != 0 || setOptStatTX != 0)
		return Lambda::Error("Connection aborted: failed to set socket timeouts", getAPIError());

	return {};
}

SOCKET BaseConnection::getHandle() {
	return this->hSocket;
}

BaseConnection& BaseConnection::operator= (BaseConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

BaseConnection::BaseConnection(BaseConnection&& other) noexcept {
	this->hSocket = other.hSocket;
	other.hSocket = INVALID_SOCKET;
}

const std::string& BaseConnection::getPeerIPv4() {
	return this->peerIPv4;
}