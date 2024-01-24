#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

Body::Body(const Body& other) {
	this->m_data = other.m_data;
}

Body::Body(const char* content) {
	this->m_data = std::vector<uint8_t>(content, content + strlen(content));
}

Body::Body(const std::string& content) {
	this->m_data = std::vector<uint8_t>(content.begin(), content.end());
}

Body::Body(const std::vector<uint8_t>& content) {
	this->m_data = content;
}

Body::operator std::string () const {
	return this->text();
}

/**
 * Returns body text reoresentation
*/
std::string Body::text() const {
	return std::string(this->m_data.begin(), this->m_data.end());
}

/**
 * Returns raw byte buffer
*/
const std::vector<uint8_t>& Body::buffer() const {
	return this->m_data;
}

/**
 * Returns body buffer size
*/
size_t Body::size() const {
	return this->m_data.size();
}
