#include "./socketsapi.hpp"
#include "./sockets.hpp"
#include <array>
#include <algorithm>

using namespace Lambda;

Socket::HTTPClientSocket::HTTPClientSocket(SOCKET hParentSocket, time_t timeoutMs) {

	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	this->hSocket = accept(hParentSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (this->hSocket == INVALID_SOCKET) {

		#ifdef _WIN32
			this->_status.error = GetLastError();
		#endif
		this->_status.code = LAMBDASOCK_ACCEPT;

		return;
	}
	
	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	auto setOptStatTX = setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	if (setOptStatRX != 0 || setOptStatTX != 0) {

		#ifdef _WIN32
			this->_status.error = GetLastError();
		#endif
		this->_status.code = LAMBDASOCK_SETOPT;

		return;
	}

	char clientIPBuff[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)) != nullptr)
		_clientIPv4 = clientIPBuff;

	this->_status.code = LAMBDASOCK_OK;
}

Socket::HTTPClientSocket::~HTTPClientSocket() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

HTTP::Request Socket::HTTPClientSocket::receiveMessage() {

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
	if (!request.headers.has("Content-Length")) return request;

	size_t bodySize;
	try { bodySize = std::stoi(request.headers.get("Content-Length")); }
		catch(...) { bodySize = 0; }

	request.body.insert(request.body.end(), headerEnded + patternEndHeader.size(), rawMessage.end());
	
	if (request.body.size() >= bodySize) return request;
	
	auto bodyChunk = std::array<uint8_t, network_chunksize_body>();
	while (request.body.size() < bodySize) {
		auto bytesReceived = recv(this->hSocket, (char*)bodyChunk.data(), bodyChunk.size(), 0);
		if (bytesReceived <= 0) break;
		request.body.insert(request.body.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	return request;
}

bool Socket::HTTPClientSocket::sendMessage(HTTP::Response& response) {

	auto payload = response.dump();

	if (send(this->hSocket, (char*)payload.data(), payload.size(), 0) <= 0) {
		this->_status.code = LAMBDASOCK_SEND;
		this->_status.error = GetLastError();
		return false;
	}

	this->_status.code = LAMBDASOCK_OK;
	this->_status.error = LAMBDASOCK_OK;
	
	return true;
}

bool Socket::HTTPClientSocket::ok() {
	return this->_status.code == LAMBDASOCK_OK;
}

Socket::SocketStatusStruct Socket::HTTPClientSocket::status() {
	return { this->_status.code, this->_status.error };
}

std::string Socket::HTTPClientSocket::ip() {
	return this->_clientIPv4;
}