#include "./crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../polyfill/polyfill.hpp"

#include <random>

using namespace Lambda;
using namespace Lambda::Crypto;

static auto rng_gen = std::mt19937(std::random_device{}());
static auto rng_dist = std::uniform_int_distribution<int32_t>(1, std::numeric_limits<int32_t>::max());

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
	this->m_id.u32 = rng_dist(rng_gen);
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
