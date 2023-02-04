#include <iostream>

#include "../include/maddsua/hex.hpp"


int main(int argc, char** argv) {

	std::string testdata_text = "Ok here is a test string";
	std::vector<uint8_t> testdata_byte = {4, 8, 16, 32, 144, 44, 17, 85};

	//	try with text
	std::cout << "Test data: \"" << testdata_text << "\"\r\n";
	auto text_hexed = maddsua::binToHex(testdata_text);
	std::cout << "HEX: \"" << text_hexed << "\"\r\n";
	auto text_restored = maddsua::hexToBin(text_hexed);

	//std::cout << "Restored: \"" << std::string(text_restored.begin(), text_restored.end()) << "\"\r\n";

	if (std::string(text_restored.begin(), text_restored.end()) != testdata_text) {
		std::cout << "Test case failed. Text mismatch\r\n";
		return 1;
	}


	//	try with numbers
	std::cout << "\r\nTest bytes:\r\n";
	for (auto number : testdata_byte) {
		std::cout << ((int)number) << " ";
	}
	auto bytes_hexed = maddsua::binToHex(testdata_byte);
	std::cout << "\r\nHEX: \"" << bytes_hexed << "\"\r\n";
	auto bytes_restored = maddsua::hexToBin(bytes_hexed);

	/*std::cout << "Restored:\r\n";
	for (auto number : bytes_restored) {
		std::cout << number << " ";
	}*/

	if (bytes_restored != testdata_byte) {
		std::cout << "Test case failed. Data mismatch\r\n";
		return 2;
	}

	std::cout << "\r\nTest ok\r\n";

	return 0;
}
