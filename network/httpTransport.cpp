#include "../lambda.hpp"
#include "./network.hpp"
#include "./tcpip.hpp"
#include "../compress/compress.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Lambda::Error Network::sendHTTPResponse(SOCKET hSocket, Response& response) {

	auto sendResponseContent = [hSocket, &response](){
		auto payloadSerialized = response.dump();
		if (send(hSocket, (char*)payloadSerialized.data(), payloadSerialized.size(), 0) <= 0)
			return Lambda::Error("Failed to send http response" , getAPIError());
		return Lambda::Error();
	};

	const auto contentEncodingHeader = response.headers.get("Content-Encoding");

	if (!contentEncodingHeader.size()) return sendResponseContent();

	if (!response.body.size()) {
		response.headers.del("Content-Encoding");
		return sendResponseContent();
	}

	const auto compressionMethod = stringTrim(contentEncodingHeader);

	std::vector<uint8_t> bodyCompressed;

	if (stringToLowerCase(compressionMethod) == "br") {

		auto compressStatus = Compress::brotliCompressBuffer(response.body, bodyCompressed);
		if (compressStatus.isError())
			return { std::string("br compression failed") + compressStatus.what() };

		response.body = bodyCompressed;

	} else if (stringToLowerCase(compressionMethod) == "gzip") {

		auto compressor = Compress::ZlibStream();
		compressor.startCompression();

		if (!compressor.compressBuffer(&response.body, &bodyCompressed))
			return { "gzip compression failed", compressor.compressionStatus() };

		response.body = bodyCompressed;

	} else if (stringToLowerCase(compressionMethod) == "deflate") {

		auto compressor = Compress::ZlibStream();
		compressor.startCompression(Compress::ZlibStream::header_deflate);

		if (!compressor.compressBuffer(&response.body, &bodyCompressed))
			return { "deflate compression failed", compressor.compressionStatus() };

		response.body = bodyCompressed;
		
	} else response.headers.del("Content-Encoding");

	return sendResponseContent();
}

/**
 * Passing body stream here is important because we can't know the header size for sure and it's very likely that we'll download some part of the body along with the headers too
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

Request Network::receiveHTTPRequest(SOCKET hSocket) {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	receiveHTTPHeader(hSocket, headerStream, bodyStream);

	auto request = Request(headerStream);	
	receiveHTTPBody(hSocket, bodyStream, request.headers.get("Content-Length"));
	request.body = bodyStream;

	return request;
}

Response Network::receiveHTTPResponse(SOCKET hSocket) {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	receiveHTTPHeader(hSocket, headerStream, bodyStream);

	auto response = Response(headerStream);
	receiveHTTPBody(hSocket, bodyStream, response.headers.get("Content-Length"));

	auto contentEncoding = response.headers.get("Content-Encoding");
	if (!contentEncoding.size()) {
		response.body = bodyStream;
		return response;
	}

	auto compressionLayers = stringSplit(static_cast<const std::string>(response.headers.get("Content-Encoding")), ",");
	std::vector<uint8_t> bodyDecompressed;

	for (auto&& encoding : compressionLayers) {

		stringTrim(encoding);

		if (encoding == "br") {

			auto decompressStatus = Compress::brotliDecompressBuffer(bodyStream, bodyDecompressed);
			if (decompressStatus.isError())
				throw Lambda::Error(std::string("br compression failed") + decompressStatus.what());

			response.body = bodyDecompressed;
			response.headers.del("Content-Encoding");

		} else if (encoding == "gzip" || encoding == "deflate") {

			auto decompressor = Compress::ZlibStream();
			decompressor.startDecompression();

			if (!decompressor.decompressBuffer(&bodyStream, &bodyDecompressed))
				throw Lambda::Error("zlib decompression failed", decompressor.compressionStatus());

			response.body = bodyDecompressed;
			response.headers.del("Content-Encoding");
		}
	}

	response.body = bodyStream;

	return response;
}
