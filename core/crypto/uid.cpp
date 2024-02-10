#include "./crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../polyfill/polyfill.hpp"

#include <chrono>

using namespace Lambda;
using namespace Lambda::Crypto;

ShortID::ShortID() {
	this->update();
}

ShortID::ShortID(uint32_t init) {
	this->update(init);
}

ShortID::ShortID(const ShortID& other) noexcept {
	this->m_id.u32 = other.m_id.u32;
	this->m_str = other.m_str;
}

ShortID::ShortID(ShortID&& other) noexcept {
	this->m_id.u32 = other.m_id.u32;
	this->m_str = std::move(other.m_str);
}

ShortID& ShortID::operator=(const ShortID& other) noexcept {
	this->m_id.u32 = other.m_id.u32;
	this->m_str = other.m_str;
	return *this;
}

void ShortID::update() noexcept {
	time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	this->m_id.u32 = (timeHighres & ~0UL) ^ (timeHighres & (static_cast<size_t>(~0UL) << 32));
	this->m_serialize();
}

void ShortID::update(uint32_t value) noexcept {
	this->m_id.u32 = value;
	this->m_serialize();
}

void ShortID::m_serialize() noexcept {

	std::string temphex;

	temphex.reserve(sizeof(this->m_id) * 2);

	for (size_t i = 0; i < sizeof(this->m_id); i++) {
		temphex.append(Encoding::byteToHex(this->m_id.buff[i]).string);
	}

	this->m_str = Strings::toLowerCase(static_cast<const std::string>(temphex));
}

uint32_t ShortID::id() const noexcept {
	return this->m_id.u32;
}

const std::string& ShortID::toString() const noexcept {
	return this->m_str;
}
