#include "./tcpip.hpp"
#include "./network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Response Network::fetch(std::string url, const RequestOptions& data) {

	//	create a connection
	struct addrinfo* resolvedAddresses = nullptr;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	auto requestUrl = URL(url);

	//	resolve host
	bool wsaInitEcexuted = false;	//	failsafe in case WSA gets glitchy
	dnsresolvehost:	//	yes, I'm using jumps here. deal with it.
	if (getaddrinfo(requestUrl.host.c_str(), requestUrl.port.c_str(), &hints, &resolvedAddresses) != 0) {
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

	SOCKET connection = INVALID_SOCKET;
	size_t connectAttempts = 0;
    for (auto addrPtr = resolvedAddresses; addrPtr != nullptr; addrPtr = addrPtr->ai_next) {

		if (connectAttempts > FETCH_MAX_ATTEMPTS) {
			auto apierror = getAPIError();
			freeaddrinfo(resolvedAddresses);
			throw Lambda::Error("Reached max attempts without succeding", apierror);
		}

		//	create socket
		connection = socket(addrPtr->ai_family, addrPtr->ai_socktype, addrPtr->ai_protocol);
		if (connection == INVALID_SOCKET) {
			auto apierror = getAPIError();
			freeaddrinfo(resolvedAddresses);
			throw Lambda::Error("Failed to create a socket", apierror);
		}

		//	connect
		if (connect(connection, addrPtr->ai_addr, addrPtr->ai_addrlen) == SOCKET_ERROR) {
			auto apierror = getAPIError();
			freeaddrinfo(resolvedAddresses);
			closesocket(connection);
			throw Lambda::Error("Failed to connect", apierror);
		}
		break;
    }

	if (connection == INVALID_SOCKET) {
		freeaddrinfo(resolvedAddresses);
		throw Lambda::Error("Could not resolve host address", getAPIError());
	}

	//	cleanup resolved addresses
	freeaddrinfo(resolvedAddresses);

	uint32_t connectionTimeout = 30000;
	auto setOptStatRX = setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&connectionTimeout, sizeof(connectionTimeout));
	auto setOptStatTX = setsockopt(connection, SOL_SOCKET, SO_SNDTIMEO, (const char*)&connectionTimeout, sizeof(connectionTimeout));
	if (setOptStatRX != 0 || setOptStatTX != 0) {
		auto errcode = getAPIError();
		throw Lambda::Error("HTTP connection aborted: failed to set socket timeouts", errcode);
	}

	//	construct http request
	std::string requestHeader = data.method + " " + requestUrl.toHttpPath() + " HTTP/1.1\r\n";

	auto requestHeaders = Headers();
	requestHeaders.fromEntries(data.headers);
	//	add some headers
	requestHeaders.set("Host", /*"http://" + */requestUrl.host);
	requestHeaders.append("User-Agent", LAMBDA_USERAGENT);
	requestHeaders.append("Accept-Encoding", LAMBDA_HTTP_ENCODINGS);
	requestHeaders.append("Accept", "*/*");

	//	append headers to request string
	requestHeader.append(requestHeaders.stringify());
	//	finish the request header with \r\n sequence
	requestHeader.append("\r\n");

	//	send request
	if (send(connection, requestHeader.data(), requestHeader.size(), 0) <= 0) {
		auto apierror = getAPIError();
		closesocket(connection);
		throw Lambda::Error("Failed to perform http request", apierror);
	}
	
	//	get response
	auto response = Network::receiveHTTPResponse(connection);

	//	cleanup
	closesocket(connection);

	return response;
}

HTTP::Response Network::fetch(std::string url) {
	return fetch(url, {});
}
