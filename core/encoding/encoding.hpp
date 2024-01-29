#ifndef __LIB_MADDSUA_LAMBDA_CORE_ENCODING__
#define __LIB_MADDSUA_LAMBDA_CORE_ENCODING__

#include <string>
#include <vector>

namespace Lambda::Encoding {

	std::string toBase64(const std::vector<uint8_t>& data);
	std::vector<uint8_t> fromBase64(const std::string& data);

	union HexByte {
		char string[3];
		struct data {
			char first;
			char second;
		} data;
	};

	HexByte byteToHex(uint8_t databyte);
	std::string toHex(const std::vector<uint8_t>& input);

	uint8_t hexToByte(HexByte hexbyte);
	std::vector<uint8_t> fromHex(const std::string& input);

	std::string encodeURIComponent(const std::string& input);
	std::string decodeURIComponent(const std::string& input);
};

#endif
