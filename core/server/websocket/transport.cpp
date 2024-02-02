#include "../server.hpp"
#include "../internal.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Server;
using namespace Lambda::Server::WSTransport;

void WebsocketContext::sendMessage(const Websocket::Message& msg) {
	auto writeBuff = serializeMessage(msg);
	this->conn.write(writeBuff);
}

FrameHeader WSTransport::parseFrameHeader(const std::vector<uint8_t>& buffer) {

	FrameHeader header {
		static_cast<FrameControlBits>(buffer.at(0) & 0xF0),
		static_cast<OpCode>(buffer.at(0) & 0x0F),
		2,
		buffer.at(1) & 0x7F
	};

	if (header.payloadSize == 126) {
		header.size += 2;
		header.payloadSize = (buffer.at(2) << 8) | buffer.at(3);
	} else if (header.payloadSize == 127) {
		header.size += 8;
		header.payloadSize = 0;
		for (int i = 0; i < 8; i++) {
			header.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
		}
	}

	bool maskUsed = (buffer.at(1) & 0x80) >> 7;
	if (maskUsed) {

		if (buffer.size() < header.size + FrameHeader::mask_size) {
			throw std::runtime_error("invalid websocket frame: not enough data to read mask");
		}

		std::array<uint8_t, FrameHeader::mask_size> mask;

		for (size_t i = 0; i < mask.size(); i++) {
			mask[i] = buffer.at(header.size + i);
		}

		header.mask = mask;
	}

	return header;
}

std::vector <uint8_t> WSTransport::serializeFrameHeader(const FrameHeader& header) {

	std::vector<uint8_t> resultBuffer;

	uint8_t finBit = static_cast<std::underlying_type_t<FrameControlBits>>(header.finbit);
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

std::vector<uint8_t> WSTransport::serializeMessage(const Message& message) {

	//	create frame buffer
	FrameHeader header {
		message.partial ? FrameControlBits::BitContinue : FrameControlBits::BitFinal,
		message.binary ? OpCode::Binary : OpCode::Text,
		message.size()	
	};
	
	auto resultBuffer = serializeFrameHeader(header);
	resultBuffer.insert(resultBuffer.end(), message.data.begin(), message.data.end());

	return resultBuffer;
}
