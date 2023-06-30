#include "./network.hpp"
#include "./tcpip.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda::HTTP;
using namespace Lambda::Network;

static const std::string websocketMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const time_t websocketRcvTimeout = 100;

//	this implementation does not support sending partial messages
//	if you want it to support it, contribute to the project, bc I personally don't need that functionality
static const uint8_t ws_finBit = 0x80;

enum WebsockBits {
	WEBSOCK_OPCODE_CONTINUE = 0x00,
	WEBSOCK_OPCODE_TEXT = 0x01,
	WEBSOCK_OPCODE_BINARY = 0x02,
	WEBSOCK_OPCODE_CLOSE = 0x08,
	WEBSOCK_OPCODE_PING = 0x09,
	WEBSOCK_OPCODE_PONG = 0x0A
};

WebSocket::WebSocket(SOCKET tcpSocket, Request& initalRequest) {

	this->hSocket = tcpSocket;

	auto headerConnection = initalRequest.headers.get("Connection");
	auto headerUpgrade = initalRequest.headers.get("Upgrade");
	auto headerWsKey = initalRequest.headers.get("Sec-WebSocket-Key");

	if (headerConnection != "Upgrade" || headerUpgrade != "websocket" || !headerWsKey.size())
		throw Lambda::Error("Websocket initialization aborted: no valid handshake headers present");

	auto combinedKey = headerWsKey + websocketMagicString;
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
		closeFrame[0] = 0x88;

		//	should always be 2 bytes, we only send a status code with no text reason
		closeFrame[1] = sizeof(connCloseStatus);

		//	the status code itself. I hate the ppl who decided not to align ws header fields. just effing masterminds.
		closeFrame[2] = (connCloseStatus >> 8) & 0xFF;
		closeFrame[3] = connCloseStatus & 0xFF;

		send(this->hSocket, (const char*)closeFrame, sizeof(closeFrame), 0);
	}

	if (this->receiveThread != nullptr) {
		if (this->receiveThread->joinable())
			this->receiveThread->join();
		delete this->receiveThread;
	}
}

void WebSocket::asyncWsIO() {

	auto setOptStatRX = setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&websocketRcvTimeout, sizeof(websocketRcvTimeout));
	if (setOptStatRX != 0) {
		auto errcode = getAPIError();
		throw Lambda::Error("Failed to set websocket receive timeout", errcode);
	}

	uint8_t downloadChunk[network_chunksize_websocket];

	WebsocketMessage* wsmessagetemp = nullptr;
	size_t messageDownloadLeft = 0;

	while (this->hSocket != INVALID_SOCKET && !this->connCloseStatus) {

		auto bytesReceived = recv(hSocket, (char*)downloadChunk, network_chunksize_websocket, 0);

		//	handle receive errors
		//	kill this websocket if connection fails
		if (bytesReceived <= 0) {

			auto apierror = getAPIError();

			if (apierror == ETIMEDOUT || apierror == WSAETIMEDOUT) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			this->asyncError = { "Websocket connection error", apierror };
			break;
		}

		if (wsmessagetemp == nullptr) {

			//	parse header
			bool mask = (downloadChunk[1] & 0x80) >> 7;
			if (!mask) throw Lambda::Error("Websock client message does not have a mask");
			
			size_t headerPayloadSize = downloadChunk[1] & 0x7F;
			size_t headerSize = 2;

			if (headerPayloadSize == 126) {
				headerPayloadSize = (downloadChunk[2] << 8) | downloadChunk[3];
				headerSize += 2;
			} else if (headerPayloadSize == 127) {
				headerPayloadSize = 0;
				for (int i = 0; i < 8; i++)
					headerPayloadSize |= (downloadChunk[2 + i] << ((7 - i) * 8));
				headerSize += 8;
			}

			uint8_t maskingKey[4];
			memcpy(maskingKey, &downloadChunk[headerSize], sizeof(maskingKey));
			headerSize += 4;

			auto payload = std::vector<uint8_t>(downloadChunk + headerSize, downloadChunk + bytesReceived);

			for (size_t i = 0; i < payload.size(); i++) {
				payload[i] ^= maskingKey[i % 4];
			}

			bool final = ((downloadChunk[0] & 0x80) >> 7);

			uint8_t opcode = downloadChunk[0] & 0x0F;

			switch (opcode) {

				case WEBSOCK_OPCODE_CLOSE: {
					this->connCloseStatus = 1000;
				} break;

				/*case WEBSOCK_OPCODE_PONG: {

				} break;

				case WEBSOCK_OPCODE_PING: {
					
					uint8_t pongFrame[8];

					// set FIN bit and opcode
					pongFrame[0] = (WEBSOCK_OPCODE_PONG | 0x80);
					//	copy payload length from ping
					pongFrame[1] = downloadChunk[1] & 0x7F;

					puts("--> ping from client");

				} break;*/

				default: {

					wsmessagetemp = new WebsocketMessage;

					wsmessagetemp->message.insert(wsmessagetemp->message.end(), payload.begin(), payload.end());
					wsmessagetemp->timestamp = time(nullptr);
					wsmessagetemp->binary = opcode == WEBSOCK_OPCODE_BINARY;
					wsmessagetemp->chunked = !final || opcode == WEBSOCK_OPCODE_CONTINUE;

					if (payload.size() < headerPayloadSize)
						messageDownloadLeft = headerPayloadSize - payload.size();
					
				} break;
			}
		}

		if (wsmessagetemp == nullptr) continue;

		if (messageDownloadLeft == 0) {

			std::lock_guard<std::mutex>lock(this->mtLock);
			this->rxQueue.push_back(*wsmessagetemp);

			delete wsmessagetemp;
			wsmessagetemp = nullptr;

			continue;
		}

		wsmessagetemp->message.insert(wsmessagetemp->message.end(), downloadChunk, downloadChunk + bytesReceived);
		if (bytesReceived < messageDownloadLeft) messageDownloadLeft -= bytesReceived;
			else messageDownloadLeft = 0;
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
	if (this->hSocket) return Lambda::Error("Socket closed | Invalid socket error");

	//	create frame buffer
	std::vector<uint8_t> frameHeader;

	// set FIN bit and opcode
	frameHeader.push_back(ws_finBit | (binary ? WEBSOCK_OPCODE_BINARY : WEBSOCK_OPCODE_TEXT));

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
