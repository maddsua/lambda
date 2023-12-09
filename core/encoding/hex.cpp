#include "../encoding.hpp"

#include <stdexcept>
#include <cstring>

using namespace Encoding;

const uint8_t hex_encode_table[] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

HexByte Encoding::encodeHexByte(char databyte) {
	HexByte result;
	result.data.first = hex_encode_table[(databyte & 0xF0) >> 4];
	result.data.second = hex_encode_table[databyte & 0x0F];
	result.string[2] = 0;
	return result;
};

std::string Encoding::toHex(const std::string& input) {

	std::string result;
	result.reserve(input.size() * 2);

	for (auto symbol : input) {
		result.append(encodeHexByte(symbol).string);
	}

	return result;
}

const uint8_t hex_decode_table[] = {
	0,1,2,3,4,5,6,7,8,9,
	0,0,0,0,0,0,0,
	10,11,12,13,14,15
};

char Encoding::decodeHexByte(HexByte hexbyte) {

	HexByte temp;

	for (size_t i = 0; i < 2; i++) {
		int charcode = hexbyte.string[i] - 48;
		if (charcode >= (int)sizeof(hex_decode_table) || charcode < 0)
			throw std::runtime_error("Invalid character in hex strng");
		temp.string[i] = hex_decode_table[charcode];
	}
	
	return (temp.data.first << 4) | temp.data.second;
}

std::string Encoding::fromHex(const std::string& input) {

	if (input.size() % 2) throw std::runtime_error("Hex string length must be divisible by 2");

	std::string result;
	result.reserve(input.size() / 2);

	for (size_t i = 0; i < input.size(); i+= 2) {
		HexByte temp;
		strncpy(temp.string, input.substr(i, 2).c_str(), sizeof(temp.string));
		result.push_back(decodeHexByte(temp));
	}

	return result;
}
