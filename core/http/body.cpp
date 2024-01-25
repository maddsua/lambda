#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

BodyBuffer::BodyBuffer(const BodyBuffer& other) {
	this->m_data = other.m_data;
}

BodyBuffer::BodyBuffer(const char* content) {
	this->m_data = std::vector<uint8_t>(content, content + strlen(content));
}

BodyBuffer::BodyBuffer(const std::string& content) {
	this->m_data = std::vector<uint8_t>(content.begin(), content.end());
}

BodyBuffer::BodyBuffer(const std::vector<uint8_t>& content) {
	this->m_data = content;
}

BodyBuffer::operator std::string () const {
	return this->text();
}

std::string BodyBuffer::text() const {
	return std::string(this->m_data.begin(), this->m_data.end());
}

const std::vector<uint8_t>& BodyBuffer::buffer() const {
	return this->m_data;
}

size_t BodyBuffer::size() const {
	return this->m_data.size();
}
