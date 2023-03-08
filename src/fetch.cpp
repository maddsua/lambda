#include "../include/maddsua/http.hpp"

maddsuaHTTP::fetchResult maddsuaHTTP::fetch(std::string url, std::string method, std::vector <maddsuaHTTP::datapair> headers, std::string body) {
	maddsuaHTTP::fetchResult result;

	std::string path;
	std::string host;
	std::string port;
	std::string fullhost;
	{
		if (url.find("https://") != std::string::npos) result.errors = "http only is supported;";

		url = std::regex_replace(url, std::regex("http(s?):\\/\\/"), "");
		url = std::regex_replace(url, std::regex("::"), ":");
		url = std::regex_replace(url, std::regex("//"), "/");

		fullhost = (url.find('/') != std::string::npos) ? url.substr(0, url.find_first_of('/')) : url;
		path = (url.find('/') != std::string::npos) ? url.substr(url.find_first_of('/')) : "/";
		host = (fullhost.find(':') != std::string::npos) ? fullhost.substr(0, fullhost.find_first_of(':')) : fullhost;
		port = (fullhost.find(':') != std::string::npos) ? fullhost.substr(fullhost.find_first_of(':')).substr(1) : "80";
	}

	//	create a connection
	struct addrinfo *hostAddr = NULL;
	struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

	//	resolve host
	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &hostAddr) != 0) {
		puts(host.c_str());
		puts(host.c_str());
		result.statusCode = 0;
		result.errors += "Failed to resolve host;";
		return result;
	}

	struct addrinfo *ptr = nullptr;
	SOCKET connection = INVALID_SOCKET;
	size_t connectAttempts = 0;
    for (ptr = hostAddr; ptr != nullptr; ptr = ptr->ai_next) {

		if (connectAttempts > MADDSUAHTTP_ATTEMPTS) {
			result.statusCode = 0;
			result.errors += "Reached max attempts without succeding;";
			return result;
		}

		//	create socket
		connection = socket(hostAddr->ai_family, hostAddr->ai_socktype, hostAddr->ai_protocol);
		if (connection == INVALID_SOCKET) {
			freeaddrinfo(hostAddr);
			result.statusCode = 0;
			result.errors += "Failed to create a socket, code " + std::to_string(WSAGetLastError()) + ";";
			return result;
		}

		//	connect
		if (connect(connection, hostAddr->ai_addr, hostAddr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(connection);
            connection = INVALID_SOCKET;
            continue;
		}
		break;
    }

	//	cleanup
	freeaddrinfo(hostAddr);

	//	send request
	if (!method.size()) method = "GET";
	if (!headerExists("Host", &headers)) headers.push_back({"Host", fullhost});

	auto rqSent = _sendData(&connection, (toUpperCase(method) + " " + path + " HTTP/1.1"), &headers, &body);
	if (!rqSent.success) {
		closesocket(connection);
		result.statusCode = 0;
		result.errors += "Failed to send request;";
		return result;
	}

	//	receive data
	auto rqResult = _getData(&connection);

	//	cleanup
	closesocket(connection);

	if (rqResult.startLineArgs.size() < 3) {
		
		result.statusCode = 0;
		result.errors += "Invalid http response;";
		return result;
	}

	try {
		result.statusCode = std::stoi(rqResult.startLineArgs[1]);
	} catch(...) {
		result.statusCode = 0;
		result.errors += "Invalid http status code;";
		return result;
	}
	
	result.statusText = _findHttpCode(result.statusCode);
	result.headers = rqResult.headers;
	result.body = rqResult.body;

	return result;
}