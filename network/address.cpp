#include "./network.hpp"
#include "../network/tcpip.hpp"

using namespace Lambda;
using namespace Lambda::Network;

SOCKET Network::resolveAndConnect(const char* host, const char* port) {
	return resolveAndConnect(host, port, CREATECONN_TCP);
}

SOCKET Network::resolveAndConnect(const char* host, const char* port, ConnectionProtocol proto) {

	struct addrinfo* resolvedAddresses = nullptr;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	switch (proto) {

		case CREATECONN_UDP:{
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
	bool wsaInitEcexuted = false;

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

		} else	//	this "else" goes to the "throw" right below, don't panic.
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
			return clientSocket;
		}

		lastError = getAPIError();
    }

	//	cleanup resolved addresses
	freeaddrinfo(resolvedAddresses);

	//	kill socket if open
	if (clientSocket != INVALID_SOCKET)
		closesocket(clientSocket);

	throw Lambda::Error("Could not connect to the server", lastError);

	return INVALID_SOCKET;
}
