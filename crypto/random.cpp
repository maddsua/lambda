#include "./crypto.hpp"
#include <random>

std::vector <uint64_t> Lambda::Crypto::randomSequence(size_t cap, size_t length) {

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

std::vector <uint8_t> Lambda::Crypto::randomStream(size_t length) {

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
