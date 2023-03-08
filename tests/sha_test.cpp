#include <iostream>

#include "../include/maddsua/crypto.hpp"


inline void print_hex(const uint8_t str[], size_t size) {
	for(size_t i = 0; i < size; i++)
		printf("%02x", str[i]);
	printf("\r\n");
}


int main(int argc, char** argv) {

	std::string data = "sample text here";


	std::cout << "Text: \"" << data << "\"\r\n";

	std::cout << "sha256: \r\n";
	auto hash = maddsua::sha256Hash(std::vector<uint8_t>(data.begin(), data.end()));
	print_hex(hash.data(), hash.size());

	std::cout << "sha512: \r\n";
	auto hash2 = maddsua::sha512Hash(std::vector<uint8_t>(data.begin(), data.end()));
	print_hex(hash2.data(), hash2.size());

	std::cout << "sha1: \r\n";
	auto hash3 = maddsua::sha1Hash(std::vector<uint8_t>(data.begin(), data.end()));
	print_hex(hash3.data(), hash3.size());

	return 0;
}
