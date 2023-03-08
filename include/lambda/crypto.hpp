/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: Cryptographic functions and other random things
*/

/*
	Required libs:
		libwinmm
*/

#ifndef H_MADDSUA_LAMBDA_CRYPTO
#define H_MADDSUA_LAMBDA_CRYPTO

	#include <stdint.h>
	#include <string>
	#include <vector>
	#include <array>
	#include <memory.h>

	#define SHA1_BLOCK_SIZE 	(20)
	#define SHA256_BLOCK_SIZE	(32)
	#define SHA512_HASH_SIZE	(64)
	#define SHA512_BLOCK_SIZE	(128)
	
	namespace lambda {

		std::array <uint8_t, SHA1_BLOCK_SIZE> sha1Hash(std::vector<uint8_t> data);
		std::array <uint8_t, SHA256_BLOCK_SIZE> sha256Hash(std::vector<uint8_t> data);
		std::array <uint8_t, SHA512_HASH_SIZE> sha512Hash(std::vector<uint8_t> data);
	}

#endif