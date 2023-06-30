#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include <array>
#include <algorithm>

using namespace Lambda::HTTP;
using namespace Lambda::Network;

HTTPConnection::HTTPConnection(SOCKET hParentSocket, time_t timeoutMs) {

	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	this->hSocket = accept(hParentSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	if (this->hSocket == INVALID_SOCKET) {
		auto errcode = getAPIError();
		throw Lambda::Exception("Socket accept failed", errcode);
	}
	
	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	auto setOptStatTX = setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMs, sizeof(timeoutMs));
	if (setOptStatRX != 0 || setOptStatTX != 0) {
		auto errcode = getAPIError();
		throw Lambda::Exception("Failed to set timeouts", errcode);
	}

	char clientIPBuff[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)) != nullptr)
		clientIPv4 = clientIPBuff;
	
}

HTTPConnection::~HTTPConnection() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}
