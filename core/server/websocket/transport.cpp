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

	header.finbit = static_cast<WebsockBits>(buffer.at(0) & 0xF0);
	header.opcode = static_cast<OpCode>(buffer.at(0) & 0x0F);

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

	bool maskUsed = (buffer.at(1) & 0x80) >> 7;
	if (maskUsed) {

		if (buffer.size() < headerOffset + WebsocketFrameHeader::mask_size) {
			throw std::runtime_error("invalid websocket frame: not enough data to read mask");
		}

		std::array<uint8_t, WebsocketFrameHeader::mask_size> mask;
		memcpy(mask.data(), buffer.data() + headerOffset, mask.size());
		header.mask = mask;
	}

	return header;
}

std::vector <uint8_t> Transport::serializeFrameHeader(const WebsocketFrameHeader& header) {

	std::vector<uint8_t> resultBuffer;

	uint8_t finBit = static_cast<std::underlying_type_t<WebsockBits>>(header.finbit);
	uint8_t opCode = static_cast<std::underlying_type_t<OpCode>>(header.opcode);

	resultBuffer.push_back(finBit | opCode);

	if (header.payloadSize < 126) {
		resultBuffer.push_back(header.payloadSize & 0x7F);
	} else if (header.payloadSize >= 126 && header.payloadSize <= 65535) {
		resultBuffer.push_back(126);
		resultBuffer.push_back((header.payloadSize >> 8) & 255);
		resultBuffer.push_back(header.payloadSize & 255);
	} else {
		resultBuffer.push_back(127);
		for (int i = 0; i < 8; i++) {
			resultBuffer.push_back((header.payloadSize >> ((7 - i) * 8)) & 0xFF);
		}
	}

	return resultBuffer;
}

std::vector<uint8_t> Transport::serializeMessage(const Message& message) {

	//	create frame buffer
	WebsocketFrameHeader header {
		message.partial ? WebsockBits::BitContinue : WebsockBits::BitFinal,
		message.binary ? OpCode::Binary : OpCode::Text,
		message.size()	
	};
	
	auto resultBuffer = serializeFrameHeader(header);
	resultBuffer.insert(resultBuffer.end(), message.data.begin(), message.data.end());

	return resultBuffer;
}
