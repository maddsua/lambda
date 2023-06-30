#include "../encoding/encoding.hpp"
#include <iostream>
#include <vector>

using namespace Lambda::Encoding;

int main() {

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

	//auto sourceString = std::string("this is a test string that must be encoded into base64..");
	auto sourceString = std::string("this is a test string.");
	//auto sourceString = std::string(binDataAsString);

	//auto encoded = b64Encode(sourceString);

	auto encoded = std::string("dGhpcyBpcyBhIHRlc3Qgc3RyaW5nL=");

	std::cout << std::endl << "Original" << std::endl;
	for (auto& c : sourceString) {
		printf("%02X ", (uint8_t)c);
	}
	printf("\n");
	

	std::cout << std::endl << "Encoded" << std::endl;
	/*for (auto& c : encoded) {
		printf(" %c ", c);
	}
	printf("\n");*/

	std::cout << encoded << std::endl;

	auto decoded = b64Decode(encoded);

	std::cout << std::endl << "Decoded" << std::endl;

	for (auto& c : decoded) {
		printf("%02X ", (uint8_t)c);
	}
	printf("\n");

	return 0;
}