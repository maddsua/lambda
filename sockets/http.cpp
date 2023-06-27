#include "socket.hpp"
#include <array>
#include <algorithm>

HTTP::Request HTTPSocket::receiveMessage(SOCKET sockref) {

	//	receive http header first
	static const std::string patternEndHeader = "\r\n\r\n";
	auto rawMessage = std::vector<uint8_t>();
	auto headerEnded = rawMessage.end();
	auto headerChunk = std::array<uint8_t, network_chunksize_header>();

	while (headerEnded == rawMessage.end()) {
		auto bytesReceived = recv(sockref, (char*)headerChunk.data(), headerChunk.size(), 0);
		if (bytesReceived <= 0) break;
		rawMessage.insert(rawMessage.end(), headerChunk.data(), headerChunk.data() + bytesReceived);
		//	"\r\n\r\n" - is a marker of http header end
		headerEnded = std::search(rawMessage.begin(), rawMessage.end(), patternEndHeader.begin(), patternEndHeader.end());
	}

	auto request = HTTP::Request(rawMessage);

	//	download request body
	if (!request.headers().has("Content-Length")) return request;

	size_t bodySize;
	try { bodySize = std::stoi(request.headers().get("Content-Length")); }
		catch(...) { bodySize = 0; }
	
	auto requestBody = std::vector<uint8_t>(headerEnded + patternEndHeader.size(), rawMessage.end());
	if (requestBody.size() >= bodySize) return request;
	
	auto bodyChunk = std::array<uint8_t, network_chunksize_body>();
	while (requestBody.size() < bodySize) {
		auto bytesReceived = recv(sockref, (char*)bodyChunk.data(), bodyChunk.size(), 0);
		if (bytesReceived <= 0) break;
		requestBody.insert(requestBody.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	request._setBody(requestBody);

	return request;
}

HTTPSocket::OpStatus HTTPSocket::sendMessage(SOCKET sockref, HTTP::Response& response) {
	auto payload = response.dump();
	if (send(sockref, (char*)payload.data(), payload.size(), 0) <= 0)
		return { HSOCKERR_SEND, GetLastError() };
	
	return { HSOCKERR_OK };
}

HTTPSocket::OpStatus HTTPSocket::createAndListen(SOCKET* sockPtr, const char* port) {

	#ifdef _WIN32
	SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (temp == INVALID_SOCKET && GetLastError() == WSANOTINITIALISED) {
		WSADATA initdata;
		auto initStat = WSAStartup(MAKEWORD(2,2), &initdata);
		if (initStat) return { HSOCKERR_INIT, GetLastError() };
			else closesocket(temp);
	}
	#endif

	//	resolve server address
	struct addrinfo* servAddr = NULL;
	struct addrinfo addrHints;
	ZeroMemory(&addrHints, sizeof(addrHints));
	addrHints.ai_family = AF_INET;
	addrHints.ai_socktype = SOCK_STREAM;
	addrHints.ai_protocol = IPPROTO_TCP;
	addrHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &addrHints, &servAddr) != 0) {
		freeaddrinfo(servAddr);
		return { HSOCKERR_LHADDRESS, GetLastError() };
	}

	// create and bind a SOCKET
	*sockPtr = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);

	if (*sockPtr == INVALID_SOCKET) {
		freeaddrinfo(servAddr);
		return { HSOCKERR_CREATE, GetLastError() };
	}

	if (bind(*sockPtr, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(*sockPtr);
		return { HSOCKERR_BIND, GetLastError() };
	}

	freeaddrinfo(servAddr);

	if (listen(*sockPtr, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(*sockPtr);
		return { HSOCKERR_LISTEN, GetLastError() };
	}

	return { HSOCKERR_OK };
}

HTTPSocket::OpStatus HTTPSocket::disconnect(SOCKET sockref) {
	
	auto status = shutdown(sockref, SD_BOTH);
	if (status) return { HSOCKERR_SHUTDOWN, GetLastError() };

	status = closesocket(sockref);
	if (status) return { HSOCKERR_CLOSESOCK, GetLastError() };

	return { HSOCKERR_OK };
}

HTTPSocket::OpStatus HTTPSocket::setConnectionTimeout(SOCKET sockref, uint32_t sockTimeoutMs) {

	auto status = setsockopt(sockref, SOL_SOCKET, SO_RCVTIMEO, (const char*)&sockTimeoutMs, sizeof(uint32_t));
	if (status) return { HSOCKERR_SENDOPT, GetLastError() };

	status = setsockopt(sockref, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sockTimeoutMs, sizeof(uint32_t));
	if (status) return { HSOCKERR_SENDOPT, GetLastError() };

	return { HSOCKERR_OK };
}
