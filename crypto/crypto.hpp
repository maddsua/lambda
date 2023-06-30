#ifndef __LAMBDA_CRYPTO__
#define __LAMBDA_CRYPTO__

	#include <stdint.h>
	#include <string>
	#include <vector>
	#include <array>
	#include <memory.h>
	
	namespace Lambda::Crypto {

		static const size_t sha1_block_size = 20;
		static const size_t sha256_block_size = 32;
		static const size_t sha512_hash_size = 64;
		static const size_t sha512_block_size = 128;

		std::array <uint8_t, sha1_block_size> sha1Hash(const std::vector<uint8_t>& data);
		std::array <uint8_t, sha256_block_size> sha256Hash(const std::vector<uint8_t>& data);
		std::array <uint8_t, sha512_hash_size> sha512Hash(const std::vector<uint8_t>& data);

		std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
		std::vector <uint8_t> randomStream(const size_t length);
	}

#endif
