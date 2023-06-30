#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"

#include <array>
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
	auto headerChunk = std::array<uint8_t, network_chunksize_header>();

	static const std::string patternEndHeader = "\r\n\r\n";

	while (headerEnded == rawMessage.end()) {

		auto bytesReceived = recv(hSocket, (char*)headerChunk.data(), headerChunk.size(), 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Exception("Network error while receiving request header", getAPIError());

		rawMessage.insert(rawMessage.end(), headerChunk.data(), headerChunk.data() + bytesReceived);
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
	
	auto bodyChunk = std::array<uint8_t, network_chunksize_body>();
	while (request.body.size() < bodySize) {

		auto bytesReceived = recv(hSocket, (char*)bodyChunk.data(), bodyChunk.size(), 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Exception("Network error while receiving request payload", getAPIError());

		request.body.insert(request.body.end(), bodyChunk.data(), bodyChunk.data() + bytesReceived);
	}

	return request;
}
