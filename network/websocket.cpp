#include "./network.hpp"
#include "./tcpip.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

#include <algorithm>
#include <array>

using namespace Lambda::HTTP;
using namespace Lambda::Network;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const std::string wslambdaPingString = "wsping/lambda";

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

	auto headerConnection = initalRequest.headers.get("Connection");
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

	auto hanshakeResult = sendHTTP(this->hSocket, handshakeReponse);

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

	size_t framesSkipped = 0;

	while (this->hSocket != INVALID_SOCKET && !this->connCloseStatus) {

		//	send ping and terminate websocket if there is no response
		auto now = std::chrono::steady_clock::now();
		if ((now - lastPong) > std::chrono::milliseconds(wsMaxSkippedAttempts * wsActTimeout)) {

			this->internalError = { "Didn't receive any response for pings" };
			this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
			return;

		} else if ((now - lastPing) > std::chrono::milliseconds(wsActTimeout)) {

			uint8_t pingFrameHeader[2];

			pingFrameHeader[0] = WEBSOCK_BIT_FINAL | WEBSOCK_OPCODE_PING;
			pingFrameHeader[1] = wslambdaPingString.size() & 0x7F;

			//	send frame header and payload in separate calls so we don't have to copy any buffers
			send(this->hSocket, (const char*)pingFrameHeader, sizeof(pingFrameHeader), 0);
			send(this->hSocket, (const char*)wslambdaPingString.data(), wslambdaPingString.size(), 0);

			lastPing = std::chrono::steady_clock::now();
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

			//	kill this websocket if it was the connection that failed
			//	but totally ignore the timeout errors
			//	we're gonna hit them constantly
			auto apierror = getAPIError();
			if (apierror != ETIMEDOUT && apierror != WSAETIMEDOUT) {
				this->internalError = { "Connection terminated", apierror };
				this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
				return;
			}

			//	don't really need this here
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
			puts("dropped a stream");
			continue;
		}

		//	Now let's ensure that we have the entire frame in the buffer
		//	I called it a stream, I don't care
		//	The entire websocket format is half-assed in my opinion, and it could've been done much better
		//	The biggest crap points are the "message fragmentation", the fact that header can have
		//	3 fucking separate header sizes and the copious amounts of bit shifting.
		//	Just make it a fucking fixed size, like 32-bit int so I can just cast it!
		//	I doubt that anyone in their rught mind would transfer more than 4GB of data in a single message,
		//	why the fuck did you decide to reinvent TCP/IP on top of TCP/IP?
		//	I mean, in "half-assed" protocol the first half is done right - the opcodes and initial idea;
		//	but the second part - the message frame format is just full of shit.
		//	I'm an fucking idiot myself, but this is beyond expert.
		//
		//	So I'm not even gonna try decoding a frame until it's fully downloaded
		if (frameHeader.size + frameHeader.payloadSize < downloadStream.size()) {

			if (framesSkipped < wsMaxSkippedAttempts) {
				this->internalError = { "Wasn't able to fetch a whole websocket frame" };
				this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
				return;
			}

			framesSkipped++;
			continue;
		}

		framesSkipped = 0;

		//	check the mask big
		if (!frameHeader.mask) {
			this->internalError = Lambda::Error("Received unmasked data from the client");
			this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
			return;
		}

		//	check opcode
		bool opcodeSupported = std::any_of(wsOpCodes.begin(), wsOpCodes.end(), [&frameHeader](auto element) {
			return element == frameHeader.opcode;
		});

		if (!opcodeSupported) {
			this->internalError = Lambda::Error("Unsupported websocket opcode");
			this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
			return;
		}

		//	ectract the payload
		//	fucking finally

		std::vector<uint8_t> payload;

		try {
			auto frameSize = (frameHeader.size + frameHeader.payloadSize);
			payload = std::vector<uint8_t>(downloadStream.begin() + frameHeader.size, downloadStream.begin() + frameSize);
			downloadStream.erase(downloadStream.begin(), downloadStream.begin() + frameSize);
		} catch(...) {
			this->internalError = Lambda::Error("Payload size mismatch");
			this->connCloseStatus = WSCLOSE_PROTOCOL_ERROR;
			return;
		}

		if (frameHeader.mask) {
			//	unmask the payload
			for (size_t i = 0; i < payload.size(); i++)
				payload[i] ^= frameHeader.maskKey[i % 4];
		}

		switch (frameHeader.opcode) {

			case WEBSOCK_OPCODE_CLOSE: {

				//	set connection close code
				//	this will cause all the operation to be shut down
				this->connCloseStatus = 1000;

			} break;

			case WEBSOCK_OPCODE_PONG: {

				//	check that pong payload matches the ping's one
				if (std::equal(payload.begin(), payload.end(), wslambdaPingString.begin(), wslambdaPingString.end())) {
					lastPong = std::chrono::steady_clock::now();
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
				puts("who the fuck said that?!");
			} break;

			default: {

				WebsocketMessage temp;
				temp.timestamp = time(nullptr);
				temp.binary = frameHeader.opcode == WEBSOCK_OPCODE_BINARY;
				temp.message.insert(temp.message.end(), payload.begin(), payload.end());

				this->rxQueue.push_back(temp);

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
