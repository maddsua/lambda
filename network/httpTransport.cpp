#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include "../compress/compress.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Lambda::Error Lambda::Network::sendHTTPResponse(SOCKET hSocket, Response& response) {

	const auto compressionMethod = stringTrim(static_cast<const std::string>(response.headers.get("Content-Encoding")));

	std::vector<uint8_t> bodyCompressed;

	if (stringToLowerCase(compressionMethod) == "br") {

		auto brStream = Compress::BrotliStream();
		brStream.startCompression();

		if (!brStream.compressBuffer(&response.body, &bodyCompressed))
			return { "br compression failed", brStream.compressionStatus() };
		
		response.body = bodyCompressed;

	} else if (stringToLowerCase(compressionMethod) == "gzip") {

		auto zlibStream = Compress::ZlibStream();
		zlibStream.startCompression();

		if (!zlibStream.compressBuffer(&response.body, &bodyCompressed))
			return { "gzip compression failed", zlibStream.compressionStatus() };

		response.body = bodyCompressed;

	} else if (stringToLowerCase(compressionMethod) == "deflate") {

		auto zlibStream = Compress::ZlibStream();
		zlibStream.startCompression(Compress::ZlibStream::header_deflate);

		if (!zlibStream.compressBuffer(&response.body, &bodyCompressed))
			return { "deflate compression failed", zlibStream.compressionStatus() };

		response.body = bodyCompressed;
	}

	auto payloadSerialized = response.dump();

	if (send(hSocket, (char*)payloadSerialized.data(), payloadSerialized.size(), 0) <= 0)
		return { "Failed to send http response" , getAPIError() };

	return {};
}

/**
 * Passing body stream here is important because we can't know the header size for sure and it's very likely that we'll fetch some part of the body too
*/
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

void receiveHTTPBody(SOCKET hSocket, std::vector<uint8_t>& bodyStream, const std::string& contentSizeHeader) {

	size_t bodySize;
	try {
		bodySize = std::stoi(contentSizeHeader);
	} catch(...) {
		bodySize = 0;
		return;
	}

	if (bodyStream.size() >= bodySize) return;
	
	uint8_t bodyChunk[network_chunksize_body];
	while (bodyStream.size() < bodySize) {

		auto bytesReceived = recv(hSocket, (char*)bodyChunk, network_chunksize_body, 0);

		if (bytesReceived == 0) break;
		else if (bytesReceived < 0) throw Lambda::Error("Network error while receiving request payload", getAPIError());

		bodyStream.insert(bodyStream.end(), bodyChunk, bodyChunk + bytesReceived);
	}
}

HTTP::Request Lambda::Network::receiveHTTPRequest(SOCKET hSocket) {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	receiveHTTPHeader(hSocket, headerStream, bodyStream);

	auto request = Request(headerStream);
	
	//	download request body
	receiveHTTPBody(hSocket, bodyStream, request.headers.get("Content-Length"));
	request.body = bodyStream;

	return request;
}
