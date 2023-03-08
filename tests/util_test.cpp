#include <iostream>

#include "../include/maddsua/crypto.hpp"
#include "../include/maddsua/util.hpp"


int main(int argc, char** argv) {

	std::cout << "Let's create a UUID: " << maddsua::createUUID() << std::endl;

	std::cout << "\r\nAnd a few random bytes:\r\n";

	auto bytes = maddsua::randomStream(5);
	for (auto num : bytes) {
		std::cout << ((int)num) << " ";
	}
	std::cout << std::endl;

	auto bytes_hexed = maddsua::binToHex(bytes);
	std::cout << "In HEX: \"" << bytes_hexed << "\"\r\n";
	auto bytes_restored = maddsua::hexToBin(bytes_hexed);

	if (bytes_restored != bytes) {
		std::cout << "Test case failed. Data mismatch\r\n";
		return 2;
	}


	auto pword = maddsua::createPassword(16, true);
	std::cout << "\r\nCreating a random password: " << pword << std::endl;
	auto pword_hexed = maddsua::binToHex(pword);
	std::cout << "In HEX: \"" << pword_hexed << "\"\r\n";
	auto pword_restored = maddsua::hexToBin(pword_hexed);

	if (std::string(pword_restored.begin(), pword_restored.end()) != pword) {
		std::cout << "Test case failed. Text mismatch\r\n";
		return 1;
	}


	std::cout << "\r\nTest OK\r\n";	

	return 0;
}
