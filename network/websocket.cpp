#include "./network.hpp"
#include "./tcpip.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

#include <algorithm>
#include <array>

using namespace Lambda::HTTP;
using namespace Lambda::Network;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const std::string wsPingString = "ping/lambda/ws";

//	The recv function blocks execution infinitely until it receives somethig,
//	which is not optimal for this usecase.
//	There's an overlapped io in winapi, but it's kinda ass and definitely is not portable
//	So I'm reinventing a wheel by causing receive function to fail quite often
//	so that at enabled the receive loop to be terminated at any time
//	It works, so fuck that, I'm not even selling this code to anyone. Yet. Remove when you do, the future Daniel.
static const time_t wsRcvTimeout = 100;

//	these values are used for both pings and actual receive timeouts
static const time_t wsActTimeout = 5000;
static const unsigned short wsMaxSkippedAttempts = 3;

//	this implementation does not support sending partial messages
//	if you want it to support it, contribute to the project, bc I personally don't need that functionality
enum WebsockBits {
	WEBSOCK_BIT_FINAL = 0x80,
	WEBSOCK_OPCODE_CONTINUE = 0x00,
	WEBSOCK_OPCODE_TEXT = 0x01,
	WEBSOCK_OPCODE_BINARY = 0x02,
	WEBSOCK_OPCODE_CLOSE = 0x08,
	WEBSOCK_OPCODE_PING = 0x09,
	WEBSOCK_OPCODE_PONG = 0x0A
};

static const std::array<uint8_t, 6> wsOpCodes = {
	WEBSOCK_OPCODE_CONTINUE,
	WEBSOCK_OPCODE_TEXT,
	WEBSOCK_OPCODE_BINARY,
	WEBSOCK_OPCODE_CLOSE,
	WEBSOCK_OPCODE_PING,
	WEBSOCK_OPCODE_PONG,
};

WebSocket::WebSocket(SOCKET tcpSocket, Request& initalRequest) {

	this->hSocket = tcpSocket;

	auto headerUpgrade = initalRequest.headers.get("Upgrade");
	auto headerWsKey = initalRequest.headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size())
		throw Lambda::Error("Websocket initialization aborted: no valid handshake headers present");

	auto combinedKey = headerWsKey + wsMagicString;
	auto keyHash = Crypto::sha1Hash(std::vector<uint8_t>(combinedKey.begin(), combinedKey.end()));
	auto keyHashString = std::string(keyHash.begin(), keyHash.end());

	auto handshakeReponse = Response(101, {
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::b64Encode(keyHashString) }
	});

	auto hanshakeResult = sendHTTPResponse(this->hSocket, handshakeReponse);

	if (hanshakeResult.isError())
		throw Lambda::Error(std::string("Websocket handshake failed: ") + hanshakeResult.what(), hanshakeResult.errorCode());

	this->receiveThread = new std::thread(asyncWsIO, this);
}

WebSocket::~WebSocket() {

	if (this->hSocket != INVALID_SOCKET && this->connCloseStatus) {

		uint8_t closeFrame[4];

		//	control frame and close opcode
		closeFrame[0] = 0x88;
		//	should always be 2 bytes, we only send a status code with no text reason
		closeFrame[1] = sizeof(connCloseStatus);
		//	the status code itself. I hate the ppl who decided not to align ws header fields. just effing masterminds.
		closeFrame[2] = (connCloseStatus >> 8) & 0xFF;
		closeFrame[3] = connCloseStatus & 0xFF;

		//	send and forget it
		send(this->hSocket, (const char*)closeFrame, sizeof(closeFrame), 0);
	}

	if (this->receiveThread != nullptr) {
		if (this->receiveThread->joinable())
			this->receiveThread->join();
		delete this->receiveThread;
	}
}

WebsocketFrameHeader WebSocket::parseFrameHeader(const std::vector<uint8_t>& buffer) {

	WebsocketFrameHeader header;

	header.finbit = (buffer.at(0) & 0x80) >> 7;
	header.opcode = buffer.at(0) & 0x0F;

	header.size = 2;
	header.payloadSize = buffer.at(1) & 0x7F;

	if (header.payloadSize == 126) {
		header.size += 2;
		header.payloadSize = (buffer.at(2) << 8) | buffer.at(3);
	} else if (header.payloadSize == 127) {
		header.size += 8;
		header.payloadSize = 0;
		for (int i = 0; i < 8; i++)
			header.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
	}

	header.mask = (buffer.at(1) & 0x80) >> 7;

	if (header.mask && buffer.size() >= header.size + sizeof(header.maskKey)) {
		memcpy(header.maskKey, buffer.data() + header.size, sizeof(header.maskKey));
		header.size += 4;
	}

	return header;
}

void clearMultipartData(WebsocketMessage** objPtr) {
	if (*objPtr == nullptr) return;
	delete *objPtr;
	*objPtr = nullptr;
}

void WebSocket::asyncWsIO() {

	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&wsRcvTimeout, sizeof(wsRcvTimeout));
	if (setOptStatRX != 0) {
		auto errcode = getAPIError();
		this->internalError = Lambda::Error("Failed to set websocket receive timeout", errcode);
		return;
	}

	uint8_t downloadChunk[network_chunksize_websocket];
	std::vector<uint8_t> downloadStream;

	auto lastPing = std::chrono::steady_clock::now();
	auto lastPong = std::chrono::steady_clock::now();

	WebsocketMessage* multipartMessagePtr = nullptr;
	uint8_t multipartMessageMask[4];

	while (this->hSocket != INVALID_SOCKET && !this->connCloseStatus) {

		//	send ping and terminate websocket if there is no response
		if ((lastPing - lastPong) > std::chrono::milliseconds(wsMaxSkippedAttempts * wsActTimeout)) {

			this->internalError = { "Didn't receive any response for pings" };
			this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: someone does not respond to the ping's! GET OUT!");
			#endif

			return;

		} else if ((std::chrono::steady_clock::now() - lastPing) > std::chrono::milliseconds(wsActTimeout)) {

			uint8_t pingFrameHeader[2];

			pingFrameHeader[0] = WEBSOCK_BIT_FINAL | WEBSOCK_OPCODE_PING;
			pingFrameHeader[1] = wsPingString.size() & 0x7F;

			//	send frame header and payload in separate calls so we don't have to copy any buffers
			send(this->hSocket, (const char*)pingFrameHeader, sizeof(pingFrameHeader), 0);
			send(this->hSocket, (const char*)wsPingString.data(), wsPingString.size(), 0);

			lastPing = std::chrono::steady_clock::now();

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: sending a ping");
			#endif

		}

		//	receive all the data available
		int32_t bytesReceived = 0;
		//	kinda stupid control flow, but I don't feel comfortable putting an infinite loop here
		while (bytesReceived >= 0) {
			bytesReceived = recv(hSocket, (char*)downloadChunk, sizeof(downloadChunk), 0);
			if (bytesReceived <= 0) break;
			downloadStream.insert(downloadStream.end(), downloadChunk, downloadChunk + bytesReceived);
		}
		
		//	if an error occured during receiving
		if (bytesReceived < 0) {

			//	kill this websocket if the connection has failed
			//	but totally ignore the timeout errors
			//	we're gonna hit them constantly
			auto apierror = getAPIError();
			if (apierror != ETIMEDOUT && apierror != WSAETIMEDOUT) {
				this->internalError = { "Connection terminated", apierror };
				this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
				return;
			}
		}

		//	skip further ops if there's no data
		if (downloadStream.size() < 2) continue;

		WebsocketFrameHeader frameHeader;

		//	try to parse ws frame
		//	it should always be a valid one
		//	but in case we receive garbage data - here is a trycatch
		try {
			frameHeader = parseFrameHeader(downloadStream);
		} catch(...) {

			//	okay, it does not seem as a valid one
			//	dropping the entire stream at this point
			downloadStream.clear();

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: dropped a stream due to invalid header");
			#endif

			continue;
		}

		//	The second-stage download loop, bc we can't rely on first-stage revc loop ⚠
		if (frameHeader.size + frameHeader.payloadSize < downloadStream.size()) {
			//	just run the download part again
			continue;
		}

		//	check the mask bit
		if (!frameHeader.mask) {

			downloadStream.clear();
			clearMultipartData(&multipartMessagePtr);

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: received unmasked data from the client");
			#endif

			continue;
		}

		//	check opcode
		bool opcodeSupported = std::any_of(wsOpCodes.begin(), wsOpCodes.end(), [&frameHeader](auto element) {
			return element == frameHeader.opcode;
		});

		if (!opcodeSupported) {

			downloadStream.clear();
			clearMultipartData(&multipartMessagePtr);

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: Unsupported websocket opcode");
			#endif

			continue;
		}

		std::vector<uint8_t> payload;

		try {
			auto frameSize = (frameHeader.size + frameHeader.payloadSize);
			payload = std::vector<uint8_t>(downloadStream.begin() + frameHeader.size, downloadStream.begin() + frameSize);
			downloadStream.erase(downloadStream.begin(), downloadStream.begin() + frameSize);
		} catch(...) {

			downloadStream.clear();
			clearMultipartData(&multipartMessagePtr);

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: std thrown a size expection, that's how bad the message is malformed");
			#endif

			return;
		}

		//	unmask the payload
		//	should actually use cpu intrinsics here
		if (frameHeader.mask && multipartMessagePtr == nullptr) {
			for (size_t i = 0; i < payload.size(); i++)
				payload[i] ^= frameHeader.maskKey[i % 4];
		} else if (frameHeader.mask) {
			for (size_t i = 0; i < payload.size(); i++)
				payload[i] ^= multipartMessageMask[i % 4];
		}

		switch (frameHeader.opcode) {

			case WEBSOCK_OPCODE_CLOSE: {

				//	set connection close code
				//	this will cause all the operation to be shut down
				this->connCloseStatus = 1000;

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: someone's leaving. bye!");
				#endif

			} break;

			case WEBSOCK_OPCODE_PONG: {

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: got a pong");
				#endif

				//	check that pong payload matches the ping's one
				if (std::equal(payload.begin(), payload.end(), wsPingString.begin(), wsPingString.end())) {

					lastPong = std::chrono::steady_clock::now();

					#ifdef LAMBDADEBUG_WS
						puts("LAMBDA_DEBUG_WS: pong valid");
					#endif
				}

			} break;

			case WEBSOCK_OPCODE_PING: {
				
				uint8_t pongFrame[2];

				// set FIN bit and opcode
				pongFrame[0] = 0x80 | WEBSOCK_OPCODE_PONG;
				//	copy payload length from ping
				pongFrame[1] = downloadChunk[1] & 0x7F;

				//	send frame header and echo the payload
				send(this->hSocket, (const char*)pongFrame, sizeof(pongFrame), 0);
				send(this->hSocket, (const char*)payload.data(), payload.size(), 0);

			} break;

			case WEBSOCK_OPCODE_CONTINUE: {

				if (multipartMessagePtr == nullptr) {
					
					downloadStream.clear();
					clearMultipartData(&multipartMessagePtr);

					#ifdef LAMBDADEBUG_WS
						puts("LAMBDA_DEBUG_WS: got continue opcode but multipart mode was not enabled. dropping the stream");
					#endif

					break;
				}

				multipartMessagePtr->content.insert(multipartMessagePtr->content.end(), payload.begin(), payload.end());

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: chewing multipart message, pls wait even more");
				#endif

				if (frameHeader.finbit) {

					this->rxQueue.push_back(*multipartMessagePtr);
					delete multipartMessagePtr;
					multipartMessagePtr = nullptr;

					#ifdef LAMBDADEBUG_WS
						puts("LAMBDA_DEBUG_WS: oh nvm, message's done");
					#endif
				}

			} break;

			default: {

				//	if that's the first frame and there are multipart leftovers - remove them
				clearMultipartData(&multipartMessagePtr);

				//	if it's a multipart message, pass the first part to temp object
				if (!frameHeader.finbit) {

					multipartMessagePtr = new WebsocketMessage;
					multipartMessagePtr->timestamp = time(nullptr);
					multipartMessagePtr->binary = frameHeader.opcode == WEBSOCK_OPCODE_BINARY;
					multipartMessagePtr->content.insert(multipartMessagePtr->content.end(), payload.begin(), payload.end());

					memcpy(multipartMessageMask, frameHeader.maskKey, sizeof(frameHeader.maskKey));

					break;
				}

				//	ok, it's not multipart, just push it to the queue
				WebsocketMessage wsMessage;
				wsMessage.timestamp = time(nullptr);
				wsMessage.binary = frameHeader.opcode == WEBSOCK_OPCODE_BINARY;
				wsMessage.content.insert(wsMessage.content.end(), payload.begin(), payload.end());

				this->rxQueue.push_back(wsMessage);

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: chewing the message, pls wait");
				#endif

			} break;
		}
	}
}

std::vector<WebsocketMessage> WebSocket::getMessages() {
	std::lock_guard<std::mutex>lock(this->mtLock);
	auto temp = this->rxQueue;
	this->rxQueue.clear();
	return temp;
}

Lambda::Error WebSocket::_sendMessage(const uint8_t* dataBuff, const size_t dataSize, bool binary) {

	if (this->connCloseStatus) return Lambda::Error("Websocket connection closed and cannot be reused");
	if (this->hSocket == INVALID_SOCKET) return Lambda::Error("Socket closed | Invalid socket error");

	//	create frame buffer
	std::vector<uint8_t> frameHeader;

	// set FIN bit and opcode
	frameHeader.push_back(WEBSOCK_BIT_FINAL | (binary ? WEBSOCK_OPCODE_BINARY : WEBSOCK_OPCODE_TEXT));

	// set payload length
	if (dataSize < 126) {
		frameHeader.push_back(dataSize & 0x7F);
	} else if (dataSize >= 126 && dataSize <= 65535) {
		frameHeader.push_back(126);
		frameHeader.push_back((dataSize >> 8) & 255);
		frameHeader.push_back(dataSize & 255);
	} else {
		frameHeader.push_back(127);
		for (int i = 0; i < 8; i++)
			frameHeader.push_back((dataSize >> ((7 - i) * 8)) & 0xFF);
	}

	//	send header
	if (send(this->hSocket, (const char*)frameHeader.data(), frameHeader.size(), 0) <= 0) return {
		"Couldn not send websocket frame header",
		getAPIError()
	};

	//	send payload
	if (send(this->hSocket, (const char*)dataBuff, dataSize, 0) <= 0) return {
		"Couldn't send ws frame payload",
		getAPIError()
	};

	return {};
}
