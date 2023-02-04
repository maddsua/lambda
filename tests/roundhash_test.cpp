#include <iostream>
#include <time.h>

#include "../include/maddsua/crypto.hpp"


inline void print_hex(const uint8_t str[], size_t size) {
	for(size_t i = 0; i < size; i++)
		printf("%02x", str[i]);
	printf("\r\n");
}


int main(int argc, char** argv) {

	std::string data = "sample text here";

	std::cout << "Hash of \"" << data << "\":\r\n";
	auto hash = maddsua::roundHash256(data);
	print_hex(hash.data(), hash.size());

	auto timeNow = std::to_string(time(nullptr));

	std::cout << "Hash of current time (" << timeNow << "):\r\n";
	hash = maddsua::roundHash256(timeNow);
	print_hex(hash.data(), hash.size());


	return 0;
}
