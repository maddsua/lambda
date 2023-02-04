/*
	Required libs:
		libwinmm
*/

#ifndef _maddsua_crypto
#define _maddsua_crypto

#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <memory.h>

#define ROUNDHASH128_SIZE	(128)
#define ROUNDHASH256_SIZE	(256)
#define ROUNDHASH512_SIZE	(512)
#define ROUNDHASH1024_SIZE	(1024)

#define SHA256_HASH_SIZE		(32)
#define SHA256_BLOCK_SIZE		(32)
#define SHA512_HASH_SIZE		(64)
#define SHA384_512_BLOCK_SIZE	(128)


	namespace maddsua {
		
		//	the "mighty" © algorithm!
		std::vector <uint8_t> _roundHash(std::vector <uint8_t> data, size_t blockSize);

		//	roundhash-128
		inline std::vector <uint8_t> roundHash128(std::vector <uint8_t> data) {
			return _roundHash(data, ROUNDHASH128_SIZE);
		}
		inline std::vector <uint8_t> roundHash128(std::string data) {
			return _roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH128_SIZE);
		}
		//	roundhash-256
		inline std::vector <uint8_t> roundHash256(std::vector <uint8_t> data) {
			return _roundHash(data, ROUNDHASH256_SIZE);
		}
		inline std::vector <uint8_t> roundHash256(std::string data) {
			return _roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH256_SIZE);
		}
		//	roundhash-512
		inline std::vector <uint8_t> roundHash512(std::vector <uint8_t> data) {
			return _roundHash(data, ROUNDHASH512_SIZE);
		}
		inline std::vector <uint8_t> roundHash512(std::string data) {
			return _roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH512_SIZE);
		}
		//	roundhash-1024
		inline std::vector <uint8_t> roundHash1024(std::vector <uint8_t> data) {
			return _roundHash(data, ROUNDHASH1024_SIZE);
		}
		inline std::vector <uint8_t> roundHash1024(std::string data) {
			return _roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH1024_SIZE);
		}
		
		//	random generators
		std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
		std::vector <uint8_t> randomStream(const size_t length);

		std::string createUUID();
		std::string createPassword(size_t length, bool randomCase);

		//	real hash functions
		std::array <uint8_t, SHA256_BLOCK_SIZE> sha256Hash(std::vector<uint8_t> data);
		std::array <uint8_t, SHA512_HASH_SIZE> sha512Hash(std::vector<uint8_t> data);
	}

#endif