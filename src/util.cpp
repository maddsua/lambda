/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/                                        


#include "../include/maddsua/crypto.hpp"

#include <random>
#include <array>
#include <time.h>
#include <windows.h>
#include <string.h>


/*

	HEX converter

*/

const uint8_t hex_table[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

std::string binToHex(const uint8_t* data, const size_t length) {
	std::string result;
		result.resize(length * 2);
	
	for (size_t m = 0, n = 0; m < length; m++, n += 2) {
		result[n] = hex_table[(data[m] & 0xF0) >> 4];
		result[n + 1] = hex_table[data[m] & 0x0F];
	}

	return result;
}


/*

	Random generators

*/

std::vector <uint64_t> lambda::randomSequence(size_t cap, size_t length) {

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

std::vector <uint8_t> lambda::randomStream(size_t length) {

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

std::array <uint8_t, UUID_BYTES> lambda::createByteUUID() {

	/*
		Byte timestamp:
		
		[-- UTC time --] [-- System time --] [--   Salt  --]
		[--  8 bytes --] [--   8 bytes   --] [-- 8 bytes --]
		<----------------     24 bytes     ---------------->

		Why not just random sequesnce of 16 bytes? Haha, that's too boring!
	*/

	std::array <uint8_t, UUID_BYTES> byteid;

	time_t utctime = time(nullptr);
	time_t systime = GetTickCount64();
	auto NaCl = randomStream(8);

	std::vector <uint8_t> timestamp;
		timestamp.resize((2 * sizeof(time_t)));
	memcpy(timestamp.data(), &utctime, sizeof(utctime));
	memcpy(timestamp.data() + sizeof(time_t), &systime, sizeof(utctime));

	timestamp.insert(timestamp.end(), NaCl.begin(), NaCl.end());
	
	auto hashbytes = sha1Hash(std::vector <uint8_t> (timestamp.begin(), timestamp.end()));
	std::copy(hashbytes.begin(), hashbytes.begin() + UUID_BYTES, byteid.begin());

	return byteid;
}

std::string lambda::formatUUID(std::array <uint8_t, UUID_BYTES>& byteid, bool showFull) {

	auto uuid = binToHex(byteid.data(), showFull ? UUID_BYTES : 4);
	if (!showFull) return uuid;

	const std::array <int, 4> uuid_separators = {8,14,19,24};
	for (auto pos : uuid_separators) {
		uuid.insert(uuid.begin() + pos, '-');
	}
	
	return uuid;
}


std::string lambda::createPassword(size_t length, bool randomCase) {

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