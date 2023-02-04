#include "../include/maddsua/crypto.hpp"
#include "../include/maddsua/hex.hpp"

#include <random>
#include <array>
#include <time.h>
#include <windows.h>

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

	auto timestamp = std::to_string(timeGetTime()) + '_' + std::to_string(time(nullptr));
	auto hash = binToHex(roundHash128(timestamp));

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