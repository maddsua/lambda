#include "./tcpip.hpp"
#include "./network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Response Lambda::Network::fetch(std::string url, const RequestOptions& data) {

	//	create a connection
	struct addrinfo *hostAddr = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	auto requestUrl = URL(url);

	//	resolve host
	if (getaddrinfo(requestUrl.host.c_str(), requestUrl.port.c_str(), &hints, &hostAddr) != 0)
		throw Lambda::Error("Failed to resolve host", getAPIError());

	struct addrinfo *ptr = nullptr;
	SOCKET connection = INVALID_SOCKET;
	size_t connectAttempts = 0;
    for (ptr = hostAddr; ptr != nullptr; ptr = ptr->ai_next) {

		if (connectAttempts > FETCH_MAX_ATTEMPTS) {
			auto apierror = getAPIError();
			freeaddrinfo(hostAddr);
			throw Lambda::Error("Reached max attempts without succeding", apierror);
		}

		//	create socket
		connection = socket(hostAddr->ai_family, hostAddr->ai_socktype, hostAddr->ai_protocol);
		if (connection == INVALID_SOCKET) {
			auto apierror = getAPIError();
			freeaddrinfo(hostAddr);
			throw Lambda::Error("Failed to create a socket", apierror);
		}

		//	connect
		if (connect(connection, hostAddr->ai_addr, hostAddr->ai_addrlen) == SOCKET_ERROR) {
			auto apierror = getAPIError();
			freeaddrinfo(hostAddr);
			closesocket(connection);
			throw Lambda::Error("Failed to connect", apierror);
		}
		break;
    }

	//	cleanup
	freeaddrinfo(hostAddr);

	std::string requestHeader = data.method + requestUrl.pathname + (requestUrl.searchParams.length() ? ("?" + requestUrl.searchParams.stringify()) : "") + "HTTP/1.1\r\n";

	auto requestHeaders = Headers();
	requestHeaders.fromEntries(data.headers);
	//	add some headers
	requestHeaders.set("Host", "http://" + requestUrl.host);
	requestHeaders.set("User-Agent", LAMBDA_USERAGENT);
	requestHeaders.set("Accept-Encoding", LAMBDA_HTTP_ENCODINGS);

	//	append to request string
	requestHeader.append(requestHeaders.stringify());

	//	send request
	if (send(connection, requestHeader.data(), requestHeader.size(), 0) < 0) {
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