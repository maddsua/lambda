#include "./network.hpp"
#include "./sysnetw.hpp"
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
static const uint16_t wsFrameMaskSize = 4;

//	this implementation does not support sending partial messages
//	if you want it to support it, contribute to the project, bc I personally don't need that functionality
enum WebsockBits {
	WEBSOCK_BIT_FINAL = 0x80,
};

static const std::array<uint8_t, 6> wsOpCodes = {
	static_cast<uint8_t>(WebSocket::Opcodes::frmcontinue),
	static_cast<uint8_t>(WebSocket::Opcodes::text),
	static_cast<uint8_t>(WebSocket::Opcodes::binary),
	static_cast<uint8_t>(WebSocket::Opcodes::ping),
	static_cast<uint8_t>(WebSocket::Opcodes::pong),
	static_cast<uint8_t>(WebSocket::Opcodes::close)
};

struct WebsocketFrameHeader {
	size_t payloadSize = 0;
	size_t size = 0;
	uint8_t maskKey[wsFrameMaskSize];
	uint8_t opcode = 0;
	bool finbit = true;
	bool mask = false;
};

WebSocket::WebSocket(HTTPConnection& connection, Request& initalRequest) {

	this->hSocket = connection.getHandle();

	try {

		auto headerUpgrade = initalRequest.headers.get("Upgrade");
		auto headerWsKey = initalRequest.headers.get("Sec-WebSocket-Key");

		if (headerUpgrade != "websocket" || !headerWsKey.size())
			throw Lambda::Error("No valid handshake headers present");

		auto combinedKey = headerWsKey + wsMagicString;

		auto sha1hash = Crypto::SHA1();
		sha1hash.update(std::vector<uint8_t>(combinedKey.begin(), combinedKey.end()));
		auto keyHash = sha1hash.digest();
		auto keyHashString = std::string(keyHash.begin(), keyHash.end());

		auto handshakeReponse = Response(101, {
			{ "Upgrade", "websocket" },
			{ "Connection", "Upgrade" },
			{ "Sec-WebSocket-Accept", Encoding::b64Encode(keyHashString) }
		});

		connection.sendResponse(handshakeReponse);

	} catch(const std::exception& e) {
		throw Lambda::Error("Websocket connection rejected", e);
	}

	this->handlerThread = std::thread(&WebSocket::asyncWsIO, this);
}

WebSocket::WebSocket(const HTTP::URL& address) {

	this->isServer = false;

	try {

		if (address.protocol != "ws")
			throw Lambda::Error("Websocket url should start with 'ws://'");

		auto connection = HTTPConnection(address);

		auto request = Request();
		request.url = address;
		request.headers.set("Connection", "Upgrade");
		request.headers.set("Upgrade", "websocket");
		request.headers.set("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
		request.headers.set("Sec-WebSocket-Version", "13");

		connection.sendRequest(request);

		auto response = connection.receiveResponse();

		if (response.statusCode() != 101)
			throw Lambda::Error("Server rejected protocol switch");

		if (response.headers.get("Connection") != "Upgrade")
			throw Lambda::Error("Server rejected connection upgrade");

		if (response.headers.get("Upgrade") != "websocket")
			throw Lambda::Error("Server rejected websocket setup");	

		this->hSocket = connection.detachHandle();
		this->handlerThread = std::thread(&WebSocket::asyncWsIO, this);

	} catch(const std::exception& e) {
		throw Lambda::Error("Unable to connect to a websocket", e);
	}	
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

	if (this->handlerThread.joinable())
		this->handlerThread.join();
}

WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer, bool isServer) {

	WebsocketFrameHeader wsfHeader;

	wsfHeader.finbit = (buffer.at(0) & 0x80) >> 7;
	wsfHeader.opcode = buffer.at(0) & 0x0F;

	wsfHeader.size = 2;
	wsfHeader.payloadSize = buffer.at(1) & 0x7F;

	if (wsfHeader.payloadSize == 126) {
		wsfHeader.size += 2;
		wsfHeader.payloadSize = (buffer.at(2) << 8) | buffer.at(3);
	} else if (wsfHeader.payloadSize == 127) {
		wsfHeader.size += 8;
		wsfHeader.payloadSize = 0;
		for (int i = 0; i < 8; i++)
			wsfHeader.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
	}

	if (!isServer) return wsfHeader;

	wsfHeader.mask = (buffer.at(1) & 0x80) >> 7;

	if (wsfHeader.mask && buffer.size() >= wsfHeader.size + wsFrameMaskSize) {
		memcpy(wsfHeader.maskKey, buffer.data() + wsfHeader.size, wsFrameMaskSize);
		wsfHeader.size += 4;
	}

	return wsfHeader;
}

std::vector<uint8_t> packFrameHeader(const WebsocketFrameHeader& header) {

	std::vector<uint8_t> result;
	result.reserve(sizeof(header));

	// set FIN bit and opcode
	result.push_back(WEBSOCK_BIT_FINAL | header.opcode);

	// set payload length
	if (header.payloadSize < 126) {
		result.push_back(header.payloadSize & 0x7F);
	} else if (header.payloadSize >= 126 && header.payloadSize <= 65535) {
		result.push_back(126);
		result.push_back((header.payloadSize >> 8) & 255);
		result.push_back(header.payloadSize & 255);
	} else {
		result.push_back(127);
		for (int i = 0; i < 8; i++)
			result.push_back((header.payloadSize >> ((7 - i) * 8)) & 0xFF);
	}

	//	set mask bs
	if (header.mask) {
		result.at(1) |= (1 << 7);
		result.resize(result.size() + wsFrameMaskSize);
		memcpy(result.data() + (result.size() - wsFrameMaskSize), header.maskKey, wsFrameMaskSize);
	}

	return result;
}

void xorMask(std::vector<uint8_t>& framePayload, uint8_t* maskKey) {
	for (size_t i = 0; i < framePayload.size(); i++)
		framePayload[i] ^= maskKey[i % 4];
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

	WebsocketMessage* multipartFrmPtr = nullptr;
	uint8_t multipartFrmMask[4];

	while (this->hSocket != INVALID_SOCKET && !this->connCloseStatus) {

		//	send ping and terminate websocket if there is no response
		if ((lastPing - lastPong) > std::chrono::milliseconds(wsMaxSkippedAttempts * wsActTimeout)) {
			this->internalError = Lambda::Error("Didn't receive any response for pings");
			this->connCloseStatus =  static_cast<uint16_t>(CloseCodes::protocol_error);
			return;

		} else if ((std::chrono::steady_clock::now() - lastPing) > std::chrono::milliseconds(wsActTimeout)) {

			sendMessage(Opcodes::ping, std::vector<uint8_t>(wsPingString.begin(), wsPingString.end()));
			lastPing = std::chrono::steady_clock::now();

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: sending a ping");
			#endif
		}

		//	receive all the data available
		int32_t bytesReceived = 0;
		while (bytesReceived >= 0) {
			
			bytesReceived = recv(hSocket, (char*)downloadChunk, sizeof(downloadChunk), 0);

			if (bytesReceived < 0) {

				auto apierror = getAPIError();
				if (apierror == ETIMEDOUT || apierror == WSAETIMEDOUT) break;
				
				this->internalError = Lambda::Error("Connection terminated", apierror);
				this->connCloseStatus = static_cast<uint16_t>(CloseCodes::protocol_error);
				return;

			} else if (bytesReceived == 0) break;

			downloadStream.insert(downloadStream.end(), downloadChunk, downloadChunk + bytesReceived);
		}

		//	skip further ops if there's no data
		if (downloadStream.size() < 2) continue;

		WebsocketFrameHeader frameHeader;

		//	try to parse ws frame
		//	it should always be a valid one
		//	but in case we receive garbage data - here is a trycatch
		try {
			frameHeader = parseFrameHeader(downloadStream, this->isServer);
		} catch(...) {
			this->internalError = Lambda::Error("Received invalid header websocket header");
			this->connCloseStatus = static_cast<uint16_t>(CloseCodes::protocol_error);
			return;
		}

		//	Run recv loop again to get all the data for sure
		if (frameHeader.size + frameHeader.payloadSize < downloadStream.size()) {
			//	just run the download part again
			continue;
		}

		//	check the mask bit
		//	Only when running as a server
		if (!frameHeader.mask && this->isServer) {
			this->internalError = Lambda::Error("Received unmasked data from the client");
			this->connCloseStatus = static_cast<uint16_t>(CloseCodes::protocol_error);
			return;
		}

		//	check opcode
		bool opcodeSupported = std::any_of(wsOpCodes.begin(), wsOpCodes.end(), [&frameHeader](auto element) {
			return element == frameHeader.opcode;
		});

		if (!opcodeSupported) {
			this->internalError = Lambda::Error("Unsupported websocket opcode");
			this->connCloseStatus = static_cast<uint16_t>(CloseCodes::protocol_error);
			return;
		}

		std::vector<uint8_t> payload;

		try {
			auto frameSize = (frameHeader.size + frameHeader.payloadSize);
			payload = std::vector<uint8_t>(downloadStream.begin() + frameHeader.size, downloadStream.begin() + frameSize);
			downloadStream.erase(downloadStream.begin(), downloadStream.begin() + frameSize);
		} catch(...) {

			downloadStream.clear();
			clearMultipartData(&multipartFrmPtr);

			#ifdef LAMBDADEBUG_WS
				puts("LAMBDA_DEBUG_WS: std thrown a size exception, that's how bad the message is malformed");
			#endif

			return;
		}

		//	unmask the payload
		if (frameHeader.mask) xorMask(payload, multipartFrmPtr == nullptr ? frameHeader.maskKey : multipartFrmMask);

		switch (frameHeader.opcode) {

			case static_cast<uint8_t>(Opcodes::close): {

				//	set connection close code
				//	this will cause all the operation to be shut down
				this->connCloseStatus = 1000;

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: someone's leaving. bye!");
				#endif

			} break;

			case static_cast<uint8_t>(Opcodes::pong): {

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

			case static_cast<uint8_t>(Opcodes::ping): {

				sendMessage(Opcodes::pong, payload);

			} break;

			case static_cast<uint8_t>(Opcodes::frmcontinue): {

				if (multipartFrmPtr == nullptr) {

					downloadStream.clear();
					clearMultipartData(&multipartFrmPtr);

					#ifdef LAMBDADEBUG_WS
						puts("LAMBDA_DEBUG_WS: got continue opcode but multipart mode was not enabled. dropping the stream");
					#endif

					break;
				}

				multipartFrmPtr->content.insert(multipartFrmPtr->content.end(), payload.begin(), payload.end());

				#ifdef LAMBDADEBUG_WS
					puts("LAMBDA_DEBUG_WS: chewing multipart message, pls wait even more");
				#endif

				if (frameHeader.finbit) {

					std::lock_guard<std::mutex>lock(this->mtLock);
					this->rxQueue.push_back(*multipartFrmPtr);

					delete multipartFrmPtr;
					multipartFrmPtr = nullptr;

					#ifdef LAMBDADEBUG_WS
						puts("LAMBDA_DEBUG_WS: oh nvm, message's done");
					#endif
				}

			} break;

			default: {

				//	if that's the first frame and there are multipart leftovers - remove them
				clearMultipartData(&multipartFrmPtr);

				//	if it's a multipart message, pass the first part to temp object
				if (!frameHeader.finbit) {

					multipartFrmPtr = new WebsocketMessage;
					multipartFrmPtr->timestamp = time(nullptr);
					multipartFrmPtr->binary = frameHeader.opcode == static_cast<uint8_t>(Opcodes::binary);
					multipartFrmPtr->content.insert(multipartFrmPtr->content.end(), payload.begin(), payload.end());

					memcpy(multipartFrmMask, frameHeader.maskKey, sizeof(frameHeader.maskKey));

					break;
				}

				//	ok, it's not multipart, just push it to the queue
				WebsocketMessage wsMessage;
				wsMessage.timestamp = time(nullptr);
				wsMessage.binary = frameHeader.opcode == static_cast<uint8_t>(Opcodes::binary);
				wsMessage.content.insert(wsMessage.content.end(), payload.begin(), payload.end());

				std::lock_guard<std::mutex>lock(this->mtLock);
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

Lambda::Error WebSocket::sendMessage(Opcodes opcode, const std::vector<uint8_t>& payload) {

	if (this->connCloseStatus)
		return Lambda::Error("Websocket connection closed and cannot be reused");

	if (this->hSocket == INVALID_SOCKET)
		return Lambda::Error("Socket closed | Invalid socket error");

	WebsocketFrameHeader header;
	header.opcode = static_cast<std::underlying_type<Opcodes>::type>(opcode);
	header.payloadSize = payload.size();

	auto sendBuffer = payload;

	if (!this->isServer) {
		header.mask = true;
		auto randomKey = Crypto::randomStream(wsFrameMaskSize);
		memcpy(header.maskKey, randomKey.data(), randomKey.size());
		xorMask(sendBuffer, header.maskKey);
	}

	auto headerPacked = packFrameHeader(header);
	sendBuffer.insert(sendBuffer.begin(), headerPacked.begin(), headerPacked.end());

	std::lock_guard<std::mutex>lock(this->mtLock);
	if (send(this->hSocket, (const char*)sendBuffer.data(), sendBuffer.size(), 0) <= 0)
		return Lambda::Error("Couldn't send websocket frame", getAPIError());

	return {};
}

Lambda::Error WebSocket::sendMessage(const std::string& message) {
	return sendMessage(Opcodes::text, std::vector<uint8_t>(message.begin(), message.end()));
}

bool WebSocket::availableMessage() {
	return this->rxQueue.size() > 0;
}

bool WebSocket::isAlive() {
	return (!this->connCloseStatus && this->hSocket != INVALID_SOCKET && !this->internalError.isError());
}
Lambda::Error WebSocket::getError() {
	return this->internalError;
}

void WebSocket::close() {
	this->connCloseStatus = static_cast<uint16_t>(CloseCodes::normal);
}

void WebSocket::close(CloseCodes reason) {
	this->connCloseStatus = static_cast<uint16_t>(reason);
}
