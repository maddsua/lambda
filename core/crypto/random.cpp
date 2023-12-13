
#include <string>
#include <array>
#include <random>

#include "../polyfill.hpp"
#include "../encoding.hpp"

using namespace Lambda;

static const std::string randomStringDict = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const std::string randomIdDict = "0123456789abcdef";
static const std::array<size_t, 5> uuidFormatSegments = { 8, 4, 4, 4, 12 };

std::vector<uint8_t> Crypto::getRandomValues(size_t length) {

	if (!length) return {};

	std::vector <uint8_t> randomBuff;
	randomBuff.resize(length);

	getRandomValues(randomBuff);
	return randomBuff;
}

void Crypto::getRandomValues(std::vector<uint8_t>& buffer) {

	if (!buffer.size()) return;

	std::random_device randev;
	std::mt19937_64 range(randev());
	std::uniform_int_distribution<std::mt19937_64::result_type> distribute(0, UINT8_MAX);

	for (size_t i = 0; i < buffer.size(); i++) {
		buffer[i] = distribute(range);
	}
}

std::string Crypto::randomUUID() {

	std::string temp;

	for (const auto& segment : uuidFormatSegments) {
		if (temp.size()) temp.push_back('-');
		temp.append(Encoding::toHex(getRandomValues(segment)));
	}

	return temp;
}

std::string Crypto::randomString(size_t length) {

	if (!length) return {};

	std::string temp;
	temp.resize(length);

	std::random_device randev;
	std::mt19937_64 range(randev());
	std::uniform_int_distribution<std::mt19937_64::result_type> distribute(0, randomStringDict.size() - 1);

	for (size_t i = 0; i < temp.size(); i++) {
		temp[i] = randomStringDict.at(distribute(range));
	}

	return temp;
}

std::string Crypto::randomID(size_t length) {

	if (!length) return {};

	std::string temp;
	temp.resize(length);

	std::random_device randev;
	std::mt19937_64 range(randev());
	std::uniform_int_distribution<std::mt19937_64::result_type> distribute(0, randomIdDict.size() - 1);

	for (size_t i = 0; i < temp.size(); i++) {
		temp[i] = randomIdDict.at(distribute(range));
	}

	return temp;
}
