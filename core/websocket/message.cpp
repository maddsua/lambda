#include "./websocket.hpp"

using namespace Lambda::Websocket;

Message::Message(const std::string& init) {
	this->m_binary = false;
	this->m_buffer = std::vector<uint8_t>(init.begin(), init.end());
}

Message::Message(const std::vector<uint8_t>& init) : m_buffer(init) {
	this->m_binary = true;
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
