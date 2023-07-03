#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include <array>
#include <algorithm>

using namespace Lambda::HTTP;
using namespace Lambda::Network;

HTTPServer::HTTPServer(SOCKET hParentSocket, time_t timeoutMs) {

	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	this->hSocket = accept(hParentSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (this->hSocket == INVALID_SOCKET) {
		auto errcode = getAPIError();
		throw Lambda::Error("HTTP connection aborted: socket accept failed", errcode);
	}
	
	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	auto setOptStatTX = setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	if (setOptStatRX != 0 || setOptStatTX != 0) {
		auto errcode = getAPIError();
		throw Lambda::Error("HTTP connection aborted: failed to set socket timeouts", errcode);
	}

	char clientIPBuff[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)) != nullptr)
		clientIPv4 = clientIPBuff;
	
}

HTTPServer::~HTTPServer() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

std::string HTTPServer::clientIP() {
	return this->clientIPv4;
}

bool HTTPServer::isAlive() {
	return this->hSocket != INVALID_SOCKET;
}

Lambda::Error HTTPServer::sendMessage(Lambda::HTTP::Response& response) {
	return sendHTTPResponse(this->hSocket, response);
}

Lambda::HTTP::Request HTTPServer::receiveMessage() {
	return receiveHTTPRequest(this->hSocket);
}

WebSocket HTTPServer::upgradeToWebsocket(Lambda::HTTP::Request& initalRequest) {
	return WebSocket(this->hSocket, initalRequest);
}

Lambda::Error Lambda::Network::HTTPServer::sendRaw(std::vector<uint8_t>& data) {
	if (send(hSocket, (char*)data.data(), data.size(), 0) <= 0)
		return { "Failed to send data" , getAPIError() };

	return {};
}
