#include "../polyfill.hpp"
#include "../encoding.hpp"

using namespace Lambda;

ShortID::ShortID(uint32_t init) {
	this->internal.numeric = init;
}

std::string ShortID::toString() const {

	std::string idstring;
	idstring.reserve(sizeof(this->internal.buffer) * 2);

	for (size_t i = 0; i < sizeof(this->internal.buffer); i++) {
		idstring.append(Encoding::byteToHex(this->internal.buffer[i]).string);
	}

	Strings::toLowerCase(idstring);
	return idstring;
}
