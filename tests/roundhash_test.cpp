#include <iostream>

#include "../include/maddsua/crypto.hpp"


inline void print_hex(const uint8_t str[], size_t size) {
	for(size_t i = 0; i < size; i++)
		printf("%02x", str[i]);
	printf("\r\n");
}


int main(int argc, char** argv) {

	auto hash = maddsua::roundHash1024("sssssssssssssssssssssssssssssssy");

	print_hex(hash.data(), hash.size());

	std::cout << hash.size();

	return 0;
}
