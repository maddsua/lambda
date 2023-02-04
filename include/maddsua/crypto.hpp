#ifndef _maddsua_crypto
#define _maddsua_crypto

#include <stdint.h>
#include <string>
#include <vector>
#include <array>

#define ROUNDHASH256_SIZE	(256)
#define ROUNDHASH512_SIZE	(512)
#define ROUNDHASH1024_SIZE	(1024)


	namespace maddsua {
		std::vector <uint8_t> roundHash(std::vector <uint8_t> data, size_t blockSize);

		inline std::vector <uint8_t> roundHash256(std::vector <uint8_t> data) {
			return roundHash(data, ROUNDHASH256_SIZE);
		}
		inline std::vector <uint8_t> roundHash256(std::string data) {
			return roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH256_SIZE);
		}
		inline std::vector <uint8_t> roundHash512(std::vector <uint8_t> data) {
			return roundHash(data, ROUNDHASH512_SIZE);
		}
		inline std::vector <uint8_t> roundHash512(std::string data) {
			return roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH512_SIZE);
		}
		inline std::vector <uint8_t> roundHash1024(std::vector <uint8_t> data) {
			return roundHash(data, ROUNDHASH1024_SIZE);
		}
		inline std::vector <uint8_t> roundHash1024(std::string data) {
			return roundHash(std::vector <uint8_t> (data.begin(), data.end()), ROUNDHASH1024_SIZE);
		}


	}

#endif