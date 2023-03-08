/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include "../include/lambda/util.hpp"
#include "../include/lambda/crypto.hpp"

#include <random>
#include <array>
#include <time.h>
#include <windows.h>
#include <string.h>

#define UUID_BYTES			(16)

/*

	HEX encoder/decoder
	Descendent of https://github.com/maddsua/base64

*/

bool b64Validate(const std::string* data) {
	
	for (auto symbol : *data) {
		if (!isalnum(symbol) && !((symbol == '+') || (symbol == '/') || (symbol == '=')))
			return false;
	}
		
	return true;
}

std::string b64Decode(std::string* data) {

	//	full decode table does brrrrrr. high-speed tricks here
	const uint8_t b64dt[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0xF8,
		0,0,0,
		0xFC,0xD0,0xD4,0xD8,0xDC,0xE0,0xE4,0xE8,0xEC,0xF0,0xF4,
		0,0,0,0,0,0,0,
		0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3C,0x40,0x44,0x48,0x4C,0x50,0x54,0x58,0x5C,0x60,0x64,
		0,0,0,0,0,0,
		0x68,0x6C,0x70,0x74,0x78,0x7C,0x80,0x84,0x88,0x8C,0x90,0x94,0x98,0x9C,0xA0,0xA4,0xA8,0xAC,0xB0,0xB4,0xB8,0xBC,0xC0,0xC4,0xC8,0xCC,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

	const size_t encodedLength = data->size();
	size_t contentLength = ((encodedLength * 3) / 4);
	
	std::string result;
		result.resize(contentLength + 3, 0);
		if (data->at(encodedLength - 1) == '=') contentLength--;
		if (data->at(encodedLength - 2) == '=') contentLength--;

	data->resize(encodedLength + 4, 0);
	
	//	even more high-speed loop than one in encoding function
	for (size_t ibase = 0, ibin = 0; ibase < encodedLength; ibase += 4, ibin += 3){
		//	byte 1/1.33
		result[ibin] = b64dt[(*data)[ibase]] ^ (b64dt[(*data)[ibase+1]] >> 6);
		//	byte 2/2.66
		result[ibin + 1] = (b64dt[(*data)[ibase+1]] << 2) ^ (b64dt[(*data)[ibase+2]] >> 4);
		//	byte 3/4
		result[ibin + 2] = (b64dt[(*data)[ibase+2]] << 4) ^ (b64dt[(*data)[ibase+3]] >> 2);
	}

	result.resize(contentLength);
	data->resize(encodedLength);

	return result;
}

std::string b64Encode(std::string* data) {

	//	yes. this table is here too
	const char b64et[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const size_t contentLength = data->size();

	std::string result;
		result.resize(((contentLength * 4) / 3) + 4, 0);
		//	+4 just adds 4 more index steps for the cycle to slide
		//	this allows us to not to make any checks inside the loop, increasing the performance
		data->resize(data->size() + 3, 0);
		//	the same story as with "encoded.resize"
				
	//	main encode loop that doe's not do any calculations but converting 8-bits to 6-bits. thats why it's so fast
	for (size_t ibin = 0, ibase = 0; ibin < contentLength; ibin += 3, ibase += 4) {
		//	byte 1/0.75
		result[ibase] = b64et[((*data)[ibin] >> 2)];
		//	byte 2/1.5
		result[ibase + 1] = b64et[(((((*data)[ibin] << 4) ^ ((*data)[ibin + 1] >> 4))) & 0x3F)];
		//	byte 3/2.25
		result[ibase + 2] = b64et[(((((*data)[ibin + 1] << 2) ^ ((*data)[ibin + 2] >> 6))) & 0x3F)];
		//	byte 4/3
		result[ibase + 3] = b64et[((*data)[ibin + 2] & 0x3F)];
	}
	
	//	very "clever" calculations
	const size_t lastBlock = (contentLength / 3) * 3;
	const short bytesRemain = (contentLength - lastBlock);
	const size_t tailBlock = (lastBlock * 4) / 3;

	if (bytesRemain > 0) {
		if (bytesRemain < 2)
			result[tailBlock + 2] = '=';

		if (bytesRemain < 3)
			result[tailBlock + 3] = '=';
	}

	result.resize(tailBlock + (bytesRemain ? 4 : 0));
	data->resize(contentLength);

	return result;
}


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

std::vector <uint64_t> randomSequence(size_t cap, size_t length) {

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

std::vector <uint8_t> randomStream(size_t length) {

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

std::string lambda::createUniqueId() {

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

	auto uuid = binToHex(byteid.data(), UUID_BYTES);

	static const std::array <int, 4> uuid_separators = {8,14,19,24};
	for (auto pos : uuid_separators)
		uuid.insert(uuid.begin() + pos, '-');
	
	return uuid;
}

/*

	Password generator

*/

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