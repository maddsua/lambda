#include "./websocket.hpp"
#include "./transport.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::WSServer;
using namespace Lambda::WSServer::Transport;

void WebsocketContext::sendMessage(const Websocket::Message& msg) {
	auto writeBuff = serializeMessage(msg);
	this->conn.write(writeBuff);
}

WebsocketFrameHeader Transport::parseFrameHeader(const std::vector<uint8_t>& buffer) {

	WebsocketFrameHeader header;

	header.finbit = (buffer.at(0) & 0x80) >> 7;
	header.opcode = buffer.at(0) & 0x0F;

	size_t headerOffset = 2;
	header.payloadSize = buffer.at(1) & 0x7F;

	if (header.payloadSize == 126) {
		headerOffset += 2;
		header.payloadSize = (buffer.at(2) << 8) | buffer.at(3);
	} else if (header.payloadSize == 127) {
		headerOffset += 8;
		header.payloadSize = 0;
		for (int i = 0; i < 8; i++) {
			header.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
		}
	}

	header.mask = (buffer.at(1) & 0x80) >> 7;

	if (header.mask && buffer.size() >= headerOffset + sizeof(header.maskKey)) {
		memcpy(header.maskKey, buffer.data() + headerOffset, sizeof(header.maskKey));
	}

	return header;
}

std::vector<uint8_t> Transport::serializeMessage(const Message& message) {

	//	create frame buffer
	std::vector<uint8_t> resultBuffer;

	// set FIN bit and opcode
	uint8_t finBit = static_cast<std::underlying_type_t<WebsockBits>>(message.partial ? WebsockBits::BitContinue : WebsockBits::BitFinal);
	uint8_t contentOpCode = static_cast<std::underlying_type_t<WebsockBits>>(message.binary ? WebsockBits::Binary : WebsockBits::Text);
	resultBuffer.push_back(finBit | contentOpCode);

	// set payload length
	const auto dataSize = message.size();
	if (dataSize < 126) {
		resultBuffer.push_back(dataSize & 0x7F);
	} else if (dataSize >= 126 && dataSize <= 65535) {
		resultBuffer.push_back(126);
		resultBuffer.push_back((dataSize >> 8) & 255);
		resultBuffer.push_back(dataSize & 255);
	} else {
		resultBuffer.push_back(127);
		for (int i = 0; i < 8; i++) {
			resultBuffer.push_back((dataSize >> ((7 - i) * 8)) & 0xFF);
		}
	}

	resultBuffer.insert(resultBuffer.end(), message.data.begin(), message.data.end());

	return resultBuffer;
}
