#include <iostream>

#include "../include/maddsua/crypto.hpp"


inline void print_hex(const uint8_t str[], size_t size) {
	for(size_t i = 0; i < size; i++)
		printf("%02x", str[i]);
	printf("\r\n");
}


int main(int argc, char** argv) {

	std::string data = "testing sha-1 woohooo";

	std::cout << "Text: \"" << data << "\"\r\n";

	std::cout << "sha1: \r\n";
	auto hash = maddsua::sha1Hash(std::vector<uint8_t>(data.begin(), data.end()));
	print_hex(hash.data(), hash.size());

	return 0;
}
