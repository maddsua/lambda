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

#define SHA1_BLOCK_SIZE 	(20)
#define SHA256_BLOCK_SIZE	(32)
#define SHA512_HASH_SIZE	(64)
#define SHA512_BLOCK_SIZE	(128)

#define UUID_BYTES		(16)

namespace lambda {

	//	random generators
	std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
	std::vector <uint8_t> randomStream(const size_t length);

	std::array <uint8_t, UUID_BYTES> createByteUUID();
	std::string formatUUID(std::array <uint8_t, UUID_BYTES>& byteid, bool showFull);
	std::string createPassword(size_t length, bool randomCase);

	//	real hash functions
	std::array <uint8_t, SHA1_BLOCK_SIZE> sha1Hash(std::vector<uint8_t> data);
	std::array <uint8_t, SHA256_BLOCK_SIZE> sha256Hash(std::vector<uint8_t> data);
	std::array <uint8_t, SHA512_HASH_SIZE> sha512Hash(std::vector<uint8_t> data);
}

#endif