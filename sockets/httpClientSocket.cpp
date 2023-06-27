#include "sockets.hpp"
#include <array>
#include <algorithm>

LambdaSocket::HTTPClientSocket::HTTPClientSocket(SOCKET hParentSocket, time_t timeoutMs) {

	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	this->hSocket = accept(hParentSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (this->hSocket == INVALID_SOCKET) {

		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_ACCEPT;

		return;
	}
	
	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	auto setOptStatTX = setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	if (setOptStatRX != 0 || setOptStatTX != 0) {

		#ifdef _WIN32
			socketError = GetLastError();
		#endif
		socketStat = HSOCKERR_SETOPT;

		return;
	}

	char clientIPBuff[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)) != nullptr)
		_clientIPv4 = clientIPBuff;

	socketStat = HSOCKERR_OK;
}

LambdaSocket::HTTPClientSocket::~HTTPClientSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

HTTP::Request LambdaSocket::HTTPClientSocket::receiveMessage() {

	//	receive http header first
	static const std::string patternEndHeader = "\r\n\r\n";
	auto rawMessage = std::vector<uint8_t>();
	auto headerEnded = rawMessage.end();
	auto headerChunk = std::array<uint8_t, network_chunksize_header>();

	while (headerEnded == rawMessage.end()) {
		auto bytesReceived = recv(this->hSocket, (char*)headerChunk.data(), headerChunk.size(), 0);
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
		auto bytesReceived = recv(this->hSocket, (char*)bodyChunk.data(), bodyChunk.size(), 0);
		if (bytesReceived <= 0) break;
		requestBody.insert(requestBody.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	request._setBody(requestBody);

	return request;
}

LambdaSocket::OpStatus LambdaSocket::HTTPClientSocket::sendMessage(HTTP::Response& response) {

	auto payload = response.dump();

	if (send(this->hSocket, (char*)payload.data(), payload.size(), 0) <= 0)
		return { HSOCKERR_SEND, GetLastError() };
	
	return { HSOCKERR_OK };
}

bool LambdaSocket::HTTPClientSocket::ok() {
	return this->socketStat == HSOCKERR_OK;
}

LambdaSocket::OpStatus LambdaSocket::HTTPClientSocket::status() {
	return { this->socketStat, this->socketError };
}

std::string LambdaSocket::HTTPClientSocket::metadata() {
	return this->_clientIPv4;
}