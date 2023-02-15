/*

	maddsua's
     ___       ________  _____ ______   ________  ________  ________
    |\  \     |\   __  \|\   _ \  _   \|\   __  \|\   ___ \|\   __  \
    \ \  \    \ \  \|\  \ \  \\\__\ \  \ \  \|\ /\ \  \_|\ \ \  \|\  \
     \ \  \    \ \   __  \ \  \\|__| \  \ \   __  \ \  \ \\ \ \   __  \
      \ \  \____\ \  \ \  \ \  \    \ \  \ \  \|\  \ \  \_\\ \ \  \ \  \
       \ \_______\ \__\ \__\ \__\    \ \__\ \_______\ \_______\ \__\ \__\
        \|_______|\|__|\|__|\|__|     \|__|\|_______|\|_______|\|__|\|__|

	A C++ HTTP server framework

	2023 https://github.com/maddsua/lambda
	
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