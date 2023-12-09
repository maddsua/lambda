
#include <cstdio>
#include <vector>
#include <exception>
#include <stdexcept>

#include "../core/encoding.hpp"

std::vector<std::pair<std::string, std::string>> test_data_base64_encode = {
	{
		"this is a test string",
		"dGhpcyBpcyBhIHRlc3Qgc3RyaW5n"
	},
	{
		"this is a test string.",
		"dGhpcyBpcyBhIHRlc3Qgc3RyaW5nLg=="
	},
	{
		"this is a test string that must be encoded into base64..",
		"dGhpcyBpcyBhIHRlc3Qgc3RyaW5nIHRoYXQgbXVzdCBiZSBlbmNvZGVkIGludG8gYmFzZTY0Li4="
	},
	{
		{
			(char)0xC9, (char)0xD0, (char)0x46, (char)0xF1,
			(char)0xAA, (char)0xB7, (char)0xBB, (char)0x7A,
			(char)0xDE, (char)0x41, (char)0xD3, (char)0x57,
			(char)0xE0, (char)0xCD, (char)0x58, (char)0x1C,
			(char)0x2A, (char)0xAD, (char)0x22, (char)0xD5
		},
		"ydBG8aq3u3reQdNX4M1YHCqtItU="
	}
};

std::vector<std::pair<std::string, std::string>> test_data_url = {
	{
		"this thing must be url encoded",
		"this%20thing%20must%20be%20url%20encoded"
	},
	{
		"\\\\||//-*",
		"%5C%5C%7C%7C%2F%2F-%2A"
	}
};

int main() {

	puts("Base64 encoding test...");
	for (auto& item : test_data_base64_encode) {
		auto temp = Encoding::toBase64(item.first);
		if (temp == item.second) continue;
		throw std::runtime_error("Unexpected encoding output: \'" + item.second + "\' is expected, but got \'" + temp + "\'");
	}
	puts("Ok\n");

	puts("Base64 decode test...");
	for (auto& item : test_data_base64_encode) {
		auto temp = Encoding::fromBase64(item.second);
		if (temp == item.first) continue;
		throw std::runtime_error("Unexpected decoding output: \'" + item.second + "\' is expected, but got \'" + temp + "\'");
	}
	puts("Ok\n");


	puts("URL encoding test...");
	for (auto& item : test_data_url) {
		auto temp = Encoding::encodeURIComponent(item.first);
		if (temp == item.second) continue;
		throw std::runtime_error("Unexpected encoding output: \'" + item.second + "\' is expected, but got \'" + temp + "\'");
	}
	puts("Ok\n");

	return 0;
}
