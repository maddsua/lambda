#include <iostream>
#include <vector>

#include "../include/lambda/util.hpp"

std::vector<std::string> tests = {
	"short",
	"longer",
	"A bit longer",
	"really long test string",
	"ok it's even longer test string"
};


int main() {

	for (auto test : tests) {

		std::cout << "\r\nTest string: \"" << test << "\"\r\n";
		
		auto encoded = b64Encode(&test);
		std::cout << "base64: \"" << encoded << "\"\r\n";

		auto decoded = b64Decode(&encoded);

		if (decoded.size() != test.size()) {
			std::cout << "Test case failed. Data size mismatch\r\n";
			return 1;
		} else if (decoded != test) {
			std::cout << "Test case failed. Text mismatch\r\n";
			return 2;
		}
	}

	std::cout << "\r\nTest ok\r\n";

	return 0;
}