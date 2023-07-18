#include "./network.hpp"
#include "./tcpip.hpp"
#include "../compress/compress.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;
using namespace Lambda::Compress;

HTTPConnection::HTTPConnection(HTTP::URL remoteUrl) {
	try {
		resolveAndConnect(remoteUrl.host.c_str(), remoteUrl.port.c_str(), ConnectionProtocol::TCP);
		setTimeouts(network_connection_timeout);
	} catch(const std::exception& e) {
		throw Lambda::Error("Failed to create HTTP connection", e);
	}
}

HTTPConnection::HTTPConnection(SOCKET hParentSocket) {

	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	this->hSocket = accept(hParentSocket, (sockaddr*)&clientAddr, &clientAddrLen);

	try {
		if (this->hSocket == INVALID_SOCKET)
			throw Lambda::Error("Socket accept failed", getAPIError());
		setTimeouts(network_connection_timeout);
	} catch(const std::exception& e) {
		throw Lambda::Error("Failed to create HTTP connection", e);
	}

	char tempbuffIPv4[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, tempbuffIPv4, sizeof(tempbuffIPv4)) != nullptr)
		peerIPv4 = tempbuffIPv4;
}

HTTPConnection::HTTPConnection(uint16_t servicePort) {
	connectLocalSerivce(servicePort, ConnectionProtocol::TCP);
	setTimeouts(network_connection_timeout);
}

void HTTPConnection::sendResponse(Response& response) {

	try {
		const auto contentEncodingHeader = response.headers.get("Content-Encoding");

		if (contentEncodingHeader.size() && response.body.size()) {

			std::vector<uint8_t> bodyCompressed;
			const auto compressionMethod = stringTrim(contentEncodingHeader);

			if (stringToLowerCase(compressionMethod) == "br") {

				auto compressStatus = brotliCompressBuffer(response.body, bodyCompressed);
				if (compressStatus.isError())
					throw Lambda::Error("br compression failed", compressStatus);

				response.body = bodyCompressed;

			} else if (stringToLowerCase(compressionMethod) == "gzip") {

				auto compressStatus = zlibCompressBuffer(response.body, bodyCompressed);
				if (compressStatus.isError())
					throw Lambda::Error("gzip compression failed", compressStatus);

				response.body = bodyCompressed;

			} else if (stringToLowerCase(compressionMethod) == "deflate") {

				auto compressStatus = zlibCompressBuffer(response.body, bodyCompressed, 5, Compress::ZLIB_HEADER_DEFLATE);
				if (compressStatus.isError())
					throw Lambda::Error("deflate compression failed", compressStatus);

				response.body = bodyCompressed;
				
			} else {
				response.headers.del("Content-Encoding");
			}

		} else if (!response.body.size()) {
			response.headers.del("Content-Encoding");
		}

		auto payloadSerialized = response.dump();

		if (send(this->hSocket, (char*)payloadSerialized.data(), payloadSerialized.size(), 0) <= 0)
			throw Lambda::Error("Network error" , getAPIError());

	} catch(const std::exception& e) {
		throw Lambda::Error("Failed to send http response", e);
	}
}

void HTTPConnection::sendRequest(HTTP::Request& request) {
	auto requestStream = request.dump();
	if (send(this->hSocket, (const char*)requestStream.data(), requestStream.size(), 0) <= 0)
		throw Lambda::Error("Failed to send http request", getAPIError());
}

/**
 * Passing body stream here is important because we can't know the header size for sure and it's very likely that we'll download some part of the body along with the headers too
*/
void receiveHeader(SOCKET hSocket, std::vector<uint8_t>& headerStream, std::vector<uint8_t>& bodyStream) {

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

void receiveBody(SOCKET hSocket, std::vector<uint8_t>& bodyStream, const std::string& contentSizeHeader) {

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

Request HTTPConnection::receiveRequest() {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	try {

		receiveHeader(this->hSocket, headerStream, bodyStream);
		auto request = Request(headerStream);

		receiveBody(this->hSocket, bodyStream, request.headers.get("Content-Length"));
		request.body = bodyStream;

		return request;

	} catch(const std::exception& e) {
		throw Lambda::Error("Could not receive http request", e);
	}
}

Response HTTPConnection::receiveResponse() {

	std::vector<uint8_t> headerStream;
	std::vector<uint8_t> bodyStream;

	try {

		receiveHeader(this->hSocket, headerStream, bodyStream);

		auto response = Response(headerStream);
		receiveBody(this->hSocket, bodyStream, response.headers.get("Content-Length"));

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
					throw Lambda::Error("brotli compression failed", decompressStatus);

				response.body = bodyDecompressed;
				response.headers.del("Content-Encoding");

			} else if (encoding == "gzip" || encoding == "deflate") {

				auto decompressStatus = Compress::zlibDecompressBuffer(bodyStream, bodyDecompressed);
				if (decompressStatus.isError())
					throw Lambda::Error("zlib compression failed", decompressStatus);

				response.body = bodyDecompressed;
				response.headers.del("Content-Encoding");

			} else {
				throw Lambda::Error("Unknown content encoding: " + encoding);
			}
		}

		response.body = bodyStream;
		return response;

	} catch(const std::exception& e) {
		throw Lambda::Error("Could not receive http response", e);
	}
}

WebSocket HTTPConnection::upgradeToWebsocket(Lambda::HTTP::Request& initalRequest) {
	return WebSocket(*this, initalRequest);
}
