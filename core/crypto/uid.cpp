#include "./crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::Crypto;

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
