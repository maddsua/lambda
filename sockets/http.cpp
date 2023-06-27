#include "socket.hpp"
#include <array>
#include <algorithm>

HTTP::Request HTTPSocket::receiveMessage(SOCKET clientSocket) {

	//	receive http header first
	static const std::string patternEndHeader = "\r\n\r\n";
	auto rawMessage = std::vector<uint8_t>();
	auto headerEnded = rawMessage.end();
	auto headerChunk = std::array<uint8_t, network_chunksize_header>();

	while (headerEnded == rawMessage.end()) {
		auto bytesReceived = recv(clientSocket, (char*)headerChunk.data(), headerChunk.size(), 0);
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
		auto bytesReceived = recv(clientSocket, (char*)bodyChunk.data(), bodyChunk.size(), 0);
		if (bytesReceived <= 0) break;
		requestBody.insert(requestBody.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	request._setBody(requestBody);

	return request;
}

bool HTTPSocket::sendMessage(SOCKET clientSocket, HTTP::Response& response) {
	auto payload = response.dump();
	auto sendResult = send(clientSocket, (char*)payload.data(), payload.size(), 0);
	return sendResult > 0;
}
