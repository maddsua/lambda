#ifndef __LAMBDA_CRYPTO__
#define __LAMBDA_CRYPTO__

	#include <stdint.h>
	#include <string>
	#include <vector>
	#include <array>
	#include <memory.h>

	#define SHA1_BLOCK_SIZE 	(20)
	#define SHA256_BLOCK_SIZE	(32)
	#define SHA512_HASH_SIZE	(64)
	#define SHA512_BLOCK_SIZE	(128)
	
	namespace Lambda::Crypto {

		std::array <uint8_t, SHA1_BLOCK_SIZE> sha1Hash(std::vector<uint8_t> data);
		std::array <uint8_t, SHA256_BLOCK_SIZE> sha256Hash(std::vector<uint8_t> data);
		std::array <uint8_t, SHA512_HASH_SIZE> sha512Hash(std::vector<uint8_t> data);

		std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
		std::vector <uint8_t> randomStream(const size_t length);
	}

#endif
