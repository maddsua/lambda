#include "../include/maddsua/hex.hpp"

#include <iostream>


const uint8_t hex_table[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};


std::string maddsua::binToHex(const uint8_t* data, const size_t length) {
	std::string result;
		result.resize(length * 2);
	
	for (size_t m = 0, n = 0; m < length; m++, n += 2) {
		result[n] = hex_table[(data[m] & 0xF0) >> 4];
		result[n + 1] = hex_table[data[m] & 0x0F];
	}

	return result;
}

std::vector <uint8_t> maddsua::hexToBin(std::string& data) {
	std::vector <uint8_t> result;
		result.resize(data.size() / 2);

	for (size_t i = 0; i < data.size(); i++) {
		if (data[i] >= 'A' && data[i] <= 'Z') data[i] += 0x20;
	}

	auto toint = [](uint8_t* dbyte) {
		if (*dbyte >= '0' && *dbyte <= '9') *dbyte -= 0x30;
		else if (*dbyte >= 'a' && *dbyte <= 'z') *dbyte -= 0x57;
	};

	for (size_t m = 0, n = 0; m < result.size(); m++, n += 2) {

		uint8_t byte_high = data[n];
		uint8_t byte_low = data[n + 1];

		toint(&byte_high);
		toint(&byte_low);

		result[m] = ((byte_high & 0x0f) << 4) | (byte_low & 0x0f);
	}
	
	return result;
}