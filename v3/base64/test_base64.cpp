#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>

#include "./base64.hpp"

#include "../utils/tests.hpp"

void test_encode_1() {

	const std::string data = "this is a test string";

	auto encoded = Lambda::Encoding::Base64::encode(std::vector<uint8_t>(data.begin(), data.end()));
	string_assert("encoded", encoded, "dGhpcyBpcyBhIHRlc3Qgc3RyaW5n");

	auto decoded = Lambda::Encoding::Base64::decode(encoded);
	string_assert("decoded", std::string(decoded.begin(), decoded.end()), data);
}

void test_encode_2() {

	const std::string data = "this is a test string.";

	auto encoded = Lambda::Encoding::Base64::encode(std::vector<uint8_t>(data.begin(), data.end()));
	string_assert("encoded", encoded, "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg==");

	auto decoded = Lambda::Encoding::Base64::decode(encoded);
	string_assert("decoded", std::string(decoded.begin(), decoded.end()), data);
}

void test_encode_3() {

	const std::string data = "this is a test string that must be encoded into base64..";

	auto encoded = Lambda::Encoding::Base64::encode(std::vector<uint8_t>(data.begin(), data.end()));
	string_assert("encoded", encoded, "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nIHRoYXQgbXVzdCBiZSBlbmNvZGVkIGludG8gYmFzZTY0Li4=");

	auto decoded = Lambda::Encoding::Base64::decode(encoded);
	string_assert("decoded", std::string(decoded.begin(), decoded.end()), data);
}

void test_encode_4() {

	const auto data = std::vector<uint8_t>({
		0xC9, 0xD0, 0x46, 0xF1,
		0xAA, 0xB7, 0xBB, 0x7A,
		0xDE, 0x41, 0xD3, 0x57,
		0xE0, 0xCD, 0x58, 0x1C,
		0x2A, 0xAD, 0x22, 0xD5
	});

	auto encoded = Lambda::Encoding::Base64::encode(data);

	string_assert("encoded", encoded, "ydBG8aq3u3reQdNX4M1YHCqtItU=");
}

int main() {

	auto queue = TestQueue({
		{ "test_encode_1", test_encode_1 },
		{ "test_encode_2", test_encode_2 },
		{ "test_encode_3", test_encode_3 },
		{ "test_encode_4", test_encode_4 },
	});

	return exec_queue(queue);
}
