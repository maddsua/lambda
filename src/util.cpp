#include "../include/maddsua/crypto.hpp"
#include "../include/maddsua/util.hpp"

#include <random>
#include <array>
#include <time.h>
#include <windows.h>


/*

	HEX converter

*/

const uint8_t hex_table[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

std::string maddsua::binToHex(const uint8_t* data, const size_t length) {
	std::string result;
		result.resize(length * 2);
	
	for (size_t m = 0, n = 0; m < length; m++, n += 2) {
		result[n] = hex_table[(data[m] & 0xF0) >> 4];
		result[n + 1] = hex_table[data[m] & 0x0F];
	}

	return result;
}

std::vector <uint8_t> maddsua::hexToBin(std::string& data) {
	std::vector <uint8_t> result;
		result.resize(data.size() / 2);

	for (size_t i = 0; i < data.size(); i++) {
		if (data[i] >= 'A' && data[i] <= 'Z') data[i] += 0x20;
	}

	auto toint = [](uint8_t* dbyte) {
		if (*dbyte >= '0' && *dbyte <= '9') *dbyte -= 0x30;
		else if (*dbyte >= 'a' && *dbyte <= 'z') *dbyte -= 0x57;
	};

	for (size_t m = 0, n = 0; m < result.size(); m++, n += 2) {

		uint8_t byte_high = data[n];
		uint8_t byte_low = data[n + 1];

		toint(&byte_high);
		toint(&byte_low);

		result[m] = ((byte_high & 0x0f) << 4) | (byte_low & 0x0f);
	}
	
	return result;
}


/*

	Random generators

*/

std::vector <uint64_t> maddsua::randomSequence(size_t cap, size_t length) {

	if (!length) return {};

	std::random_device dev;
	std::mt19937_64 rng(dev());
	std::uniform_int_distribution<std::mt19937_64::result_type> dist6(0, cap);

	std::vector <size_t> randomIntList;
		randomIntList.resize(length);

	for (auto& random : randomIntList)
		random = dist6(rng);

	return randomIntList;
}

std::vector <uint8_t> maddsua::randomStream(size_t length) {

	if (!length) return {};

	std::random_device dev;
	std::mt19937_64 rng(dev());
	std::uniform_int_distribution<std::mt19937_64::result_type> dist6(0, UINT8_MAX);

	std::vector <uint8_t> randomIntList;
		randomIntList.resize(length);

	for (auto& random : randomIntList)
		random = dist6(rng);

	return randomIntList;
}

std::string maddsua::createUUID() {

	auto timestamp = std::to_string(timeGetTime()) + std::to_string(time(nullptr));

	auto digest = sha1Hash(std::vector <uint8_t> (timestamp.begin(), timestamp.end()));

	auto hash = binToHex(digest.data(), UUID_BSIZETR);

	const std::array <int, 4> uuid_separators = {8,14,19,24};
	for (auto pos : uuid_separators) {
		hash.insert(hash.begin() + pos, '-');
	}
	
	return hash;
}

std::string maddsua::createPassword(size_t length, bool randomCase) {

	std::string alnum = "qwertyuiopasdfghjklzxcvbnm0123456789";

	std::string password;
		password.reserve(length);

	auto indexes = randomSequence(alnum.size() - 1, length);
	for (auto index : indexes)
		password.push_back(alnum[index]);

	if (randomCase) {
		auto caseModifiers = randomSequence(1, length);
		for (size_t i = 0; i < length; i++) {
			if (password[i] >= 'a' && password[i] <= 'z' && caseModifiers[i]) {
				password[i] -= 0x20;
			}
		}
	}
	
	return password;
}