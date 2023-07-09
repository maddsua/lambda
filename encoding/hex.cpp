#include "./encoding.hpp"

using namespace Lambda;

const uint8_t hex_table[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

std::string binToHex(const uint8_t* data, const size_t length) {
	std::string result;
		result.resize(length * 2);
	
	for (size_t m = 0, n = 0; m < length; m++, n += 2) {
		result[n] = hex_table[(data[m] & 0xF0) >> 4];
		result[n + 1] = hex_table[data[m] & 0x0F];
	}

	return result;
}

std::string Encoding::hexEncode(std::string data) {
	return binToHex((uint8_t*)data.data(), data.size());
}
std::string Encoding::hexEncode(std::vector<uint8_t> data) {
	return binToHex(data.data(), data.size());
}
