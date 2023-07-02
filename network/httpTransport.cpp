#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Lambda::Error Lambda::Network::sendHTTP(SOCKET hSocket, Response& response) {

	auto payloadSerialized = response.dump();

	if (send(hSocket, (char*)payloadSerialized.data(), payloadSerialized.size(), 0) <= 0)
		return { "Failed to send http response" , getAPIError() };

	return {};
}

HTTP::Request Lambda::Network::receiveHTTP(SOCKET hSocket) {

	auto rawMessage = std::vector<uint8_t>();
	auto headerEnded = rawMessage.end();
	uint8_t headerChunk[network_chunksize_header];

	static const std::string patternEndHeader = "\r\n\r\n";

	while (headerEnded == rawMessage.end()) {

		auto bytesReceived = recv(hSocket, (char*)headerChunk, sizeof(headerChunk), 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Error("Network error while receiving request header", getAPIError());

		rawMessage.insert(rawMessage.end(), headerChunk, headerChunk + bytesReceived);
		headerEnded = std::search(rawMessage.begin(), rawMessage.end(), patternEndHeader.begin(), patternEndHeader.end());
	}

	auto request = Request(rawMessage);
	request.body.insert(request.body.end(), headerEnded + patternEndHeader.size(), rawMessage.end());
	
	//	download request body
	size_t bodySize;
	try {
		bodySize = std::stoi(request.headers.get("Content-Length"));
	} catch(...) {
		bodySize = 0;
		return request;
	}

	if (request.body.size() >= bodySize) return request;
	
	uint8_t bodyChunk[network_chunksize_body];
	while (request.body.size() < bodySize) {

		auto bytesReceived = recv(hSocket, (char*)bodyChunk, network_chunksize_body, 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Error("Network error while receiving request payload", getAPIError());

		request.body.insert(request.body.end(), bodyChunk, bodyChunk + bytesReceived);
	}

	return request;
}

Lambda::Error Lambda::Network::HTTPConnection::sendRaw(std::vector<uint8_t>& data) {
	if (send(hSocket, (char*)data.data(), data.size(), 0) <= 0)
		return { "Failed to send data" , getAPIError() };

	return {};
}