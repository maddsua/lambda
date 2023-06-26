#include "socket.hpp"
#include <array>
#include <algorithm>

HTTP::Request HTTPSocket::receive(SOCKET* client) {

	//	receive http header first
	static const std::string patternEndHeader = "\r\n\r\n";
	auto rawMessage = std::vector<uint8_t>();
	auto headerEnded = rawMessage.end();
	auto headerChunk = std::array<uint8_t, network_chunksize_header>();

	while (headerEnded == rawMessage.end()) {
		auto bytesReceived = recv(*client, (char*)headerChunk.data(), headerChunk.size(), 0);
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
	if (requestBody.size() >= bodySize) return;
	
	auto bodyChunk = std::array<uint8_t, network_chunksize_body>();
	while (requestBody.size() < bodySize) {
		auto bytesReceived = recv(*client, (char*)bodyChunk.data(), bodyChunk.size(), 0);
		if (bytesReceived <= 0) break;
		requestBody.insert(requestBody.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	request._setBody(requestBody);

	return request;
}
