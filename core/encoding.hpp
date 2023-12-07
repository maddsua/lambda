#ifndef _OCTOPUSS_CORE_ENCODING_
#define _OCTOPUSS_CORE_ENCODING_

#include "./polyfill.hpp"
#include "./http.hpp"

namespace Encoding {

	std::string toBase64(const std::string& data);
	std::string fromBase64(const std::string& data);

	union HexByte {
		char string[3];
		struct data {
			char first;
			char second;
		} data;
	};

	HexByte encodeHexByte(char databyte);
	std::string toHex(const std::string& input);

	char decodeHexByte(HexByte hexbyte);
	std::string fromHex(const std::string& input);

	std::string encodeURIComponent(const std::string input);
	std::string decodeURIComponent(const std::string input);
};

#endif
