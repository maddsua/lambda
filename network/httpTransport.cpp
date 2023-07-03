#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Lambda::Error Lambda::Network::sendHTTPResponse(SOCKET hSocket, Response& response) {

	auto payloadSerialized = response.dump();

	if (send(hSocket, (char*)payloadSerialized.data(), payloadSerialized.size(), 0) <= 0)
		return { "Failed to send http response" , getAPIError() };

	return {};
}

void receiveHTTPHeader(SOCKET hSocket, std::vector<uint8_t>& headerStream, std::vector<uint8_t>& bodyStream) {

	auto headerEnded = headerStream.end();
	uint8_t headerChunk[network_chunksize_header];

	static const std::string patternEndHeader = "\r\n\r\n";

	while (headerEnded == headerStream.end()) {

		auto bytesReceived = recv(hSocket, (char*)headerChunk, sizeof(headerChunk), 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Error("Network error while receiving request header", getAPIError());

		headerStream.insert(headerStream.end(), headerChunk, headerChunk + bytesReceived);
		headerEnded = std::search(headerStream.begin(), headerStream.end(), patternEndHeader.begin(), patternEndHeader.end());
	}

	if (headerEnded == headerStream.end()) throw Lambda::Error("Invalid http header: no trailing sequence");

	bodyStream.insert(bodyStream.end(), headerEnded + patternEndHeader.size(), headerStream.end());
	headerStream.resize(headerEnded - headerStream.begin());
}

HTTP::Request Lambda::Network::receiveHTTPRequest(SOCKET hSocket) {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	receiveHTTPHeader(hSocket, headerStream, bodyStream);

	auto request = Request(headerStream);
	
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
