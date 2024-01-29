#include "./crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../polyfill/polyfill.hpp"

#include <chrono>

using namespace Lambda::Crypto;

ShortID::ShortID() {
	time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	this->m_id.numeric = (timeHighres & ~0UL);
}

ShortID::ShortID(uint32_t init) {
	this->m_id.numeric = init;
}

std::string ShortID::toString() const {

	std::string idstring;
	idstring.reserve(sizeof(this->m_id.buffer) * 2);

	for (size_t i = 0; i < sizeof(this->m_id.buffer); i++) {
		idstring.append(Encoding::byteToHex(this->m_id.buffer[i]).string);
	}

	Strings::toLowerCase(idstring);
	return idstring;
}
