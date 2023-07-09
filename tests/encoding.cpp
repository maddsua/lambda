#include "../encoding/encoding.hpp"
#include <iostream>
#include <vector>

using namespace Lambda::Encoding;

int main() {

	std::cout << "\r\n--- base64 encoding/decoding test begin --- \r\n";

	std::vector<uint8_t> binaryData = {
		0xC9,
		0xD0,
		0x46,
		0xF1,
		0xAA,
		0xB7,
		0xBB,
		0x7A,
		0xDE,
		0x41,
		0xD3,
		0x57,
		0xE0,
		0xCD,
		0x58,
		0x1C,
		0x2A,
		0xAD,
		0x22,
		0xD5
	};

	auto binDataAsString = std::string(binaryData.begin(), binaryData.end());

	auto sourceString = std::string("this is a test string that must be encoded into base64..");

	auto encoded = b64Encode(sourceString);

	std::cout << std::endl << "Original: " << sourceString << std::endl;
	for (auto& c : sourceString) {
		printf("%02X ", (uint8_t)c);
	}
	printf("\n");
	

	std::cout << std::endl << "Encoded" << std::endl;

	std::cout << encoded << std::endl;

	auto decoded = b64Decode(encoded);

	std::cout << std::endl << "Decoded: " << decoded << std::endl;

	for (auto& c : decoded) {
		printf("%02X ", (uint8_t)c);
	}
	printf("\n");

	return 0;
}