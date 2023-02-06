#include <iostream>

#define LAMBDADEBUG


#include "../include/maddsua/lambda.hpp"
#include "../include/maddsua/crypto.hpp"


int main(int argc, char** argv) {

	auto byteuuid = lambda::createByteUUID();
	std::cout << "Let's create a UUID: " << lambda::formatUUID(byteuuid, true) << std::endl;

	std::cout << "\r\nAnd a few random bytes:\r\n";

	auto bytes = lambda::randomStream(5);
	for (auto num : bytes) {
		std::cout << ((int)num) << " ";
	}
	std::cout << std::endl;

	auto bytes_hexed = binToHex(bytes);
	std::cout << "In HEX: \"" << bytes_hexed << "\"\r\n";
	auto bytes_restored = hexToBin(bytes_hexed);

	if (bytes_restored != bytes) {
		std::cout << "Test case failed. Data mismatch\r\n";
		return 2;
	}


	auto pword = lambda::createPassword(16, true);
	std::cout << "\r\nCreating a random password: " << pword << std::endl;
	auto pword_hexed = binToHex(pword);
	std::cout << "In HEX: \"" << pword_hexed << "\"\r\n";
	auto pword_restored = hexToBin(pword_hexed);

	if (std::string(pword_restored.begin(), pword_restored.end()) != pword) {
		std::cout << "Test case failed. Text mismatch\r\n";
		return 1;
	}


	std::cout << "\r\nTest OK\r\n";	

	return 0;
}
