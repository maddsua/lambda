#include "./websocket.hpp"

using namespace Lambda::Websocket;

struct WebsocketFrameHeader {
	size_t payloadSize;
	uint8_t maskKey[4];
	uint8_t opcode;
	bool finbit;
	bool mask;
};

WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer) {

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
		for (int i = 0; i < 8; i++)
			header.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
	}

	header.mask = (buffer.at(1) & 0x80) >> 7;

	if (header.mask && buffer.size() >= headerOffset + sizeof(header.maskKey)) {
		memcpy(header.maskKey, buffer.data() + headerOffset, sizeof(header.maskKey));
	}

	return header;
}
