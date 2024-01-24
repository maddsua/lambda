#include "./websocket.hpp"
#include "./internal.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

Message::Message(const std::string& init) {
	this->m_binary = false;
	this->m_buffer = std::vector<uint8_t>(init.begin(), init.end());
}

Message::Message(const std::string& init, bool partial) {
	this->m_binary = false;
	this->m_partial = partial;
	this->m_buffer = std::vector<uint8_t>(init.begin(), init.end());
}

Message::Message(const std::vector<uint8_t>& init) : m_buffer(init) {
	this->m_binary = true;
}

Message::Message(const std::vector<uint8_t>& init, bool partial) : m_buffer(init) {
	this->m_binary = true;
	this->m_partial = partial;
}

const std::vector<uint8_t>& Message::data() const noexcept {
	return this->m_buffer;
}

std::string Message::text() const {
	return std::string(this->m_buffer.begin(), this->m_buffer.end());
}

bool Message::isBinary() const noexcept {
	return this->m_binary;
}

bool Message::isPartial() const noexcept {
	return this->m_partial;
}

size_t Message::size() const noexcept {
	this->data().size();
}

WebsocketFrameHeader Websocket::parseFrameHeader(const std::vector<uint8_t>& buffer) {

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

std::vector <uint8_t> Websocket::serializeMessage(const Message& message) {

	//	create frame buffer
	std::vector<uint8_t> resultBuffer;

	// set FIN bit and opcode
	uint8_t finBit = static_cast<std::underlying_type_t<WebsockBits>>(message.isPartial() ? WebsockBits::BitContinue : WebsockBits::BitFinal);
	uint8_t contentOpCode = static_cast<std::underlying_type_t<WebsockBits>>(message.isBinary() ? WebsockBits::Binary : WebsockBits::Text);
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

	const auto& messageData = message.data();
	resultBuffer.insert(resultBuffer.end(), messageData.begin(), messageData.end());

	return resultBuffer;
}
