#include "../include/maddsua/crypto.hpp"

#include <memory.h>

#define HASH_ROUNDS			(128)		//	128 hashing rounds

const uint8_t hash_iv[128] = {
	//	0-512 bits
	0x80,0xd8,0xfa,0x4c,0xf9,0x60,0x40,0x8,
	0xf8,0x41,0x63,0xa,0xa9,0xc6,0xb5,0xe1,
	0x72,0xd2,0x23,0xc6,0x7,0x1f,0x91,0x57,
	0xe5,0x5e,0x37,0x8e,0x7b,0x2f,0x90,0x26,
	0x81,0x8f,0xd9,0xde,0xa2,0xbe,0x87,0x27,
	0x27,0x9a,0xc,0xf8,0xe2,0x81,0xc3,0x19,
	0x7f,0x20,0x66,0xff,0x49,0x8f,0x0,0x42,
	0x98,0x2c,0x63,0x86,0xbe,0x64,0x1b,0xf4,
	//	512-1024 bits
	0xfe,0x2d,0xe9,0xfb,0x82,0x36,0x42,0x7c,
	0x5,0x7d,0xce,0xbf,0xde,0xdc,0x32,0x95,
	0xdc,0x24,0x1c,0x9a,0x82,0x1f,0xa9,0x5e,
	0x5d,0x8e,0xc6,0xa9,0x93,0xb6,0x28,0x33,
	0x38,0x1,0x9d,0x70,0x97,0x9b,0xe6,0x6c,
	0xd7,0x4c,0x2f,0xbd,0x95,0x9,0xc3,0x87,
	0x42,0x2,0x9,0x6,0xaf,0xb5,0x52,0x9a,
	0x4f,0x74,0xbf,0x4,0xd5,0x6a,0x7e,0xbc
};

void xorTo(uint8_t* data_A, const uint8_t* data_B, const size_t blockSize) {
	for (size_t i = 0; i < blockSize; i++)
		data_A[i] ^= data_B[i];
}

void hashBlock(uint8_t* data, const size_t blockSize) {

	for (size_t i = 0; i < HASH_ROUNDS; i++) {

		//	xor data to itself
		for (size_t m = 0; m < blockSize; m++) {
			for (size_t n = 0; n < blockSize; n++)
				data[m] ^= data[n];
		}

		//	adaptive shift
		uint8_t swapLayers[blockSize][blockSize];
		memset(swapLayers, 0, blockSize * blockSize);

		for (size_t m = 0; m < blockSize; m++) {
			for (size_t n = 0; n < blockSize; n++) {
				int8_t shi = n + ((((float)data[n] / blockSize) - (int)((float)data[n] / blockSize)) * blockSize);
					if (shi >= blockSize) shi -= blockSize;
					else if (shi < 0) shi += blockSize;
				swapLayers[m][shi] = data[n];
			}
		}

		for (size_t m = 0; m < blockSize; m++) {
			for (size_t n = 0; n < blockSize; n++)
				data[m] ^= swapLayers[m][n];
		}

		//	shift bits in this round while xoring them to the next bit and to iv value
		uint8_t fstb = data[0];
		for (size_t i = 0; i < blockSize - 1; i++)
			data[i] = data[i + 1] ^ hash_iv[i] ^ data[i] ^ hash_iv[i + 1];

		data[blockSize - 1] = (fstb ^ hash_iv[blockSize - 1]) ^ (data[3] << 6) ^ (data[6] << 3);
	}
}

std::vector <uint8_t> maddsua::_roundHash(std::vector <uint8_t> data, size_t blockSize) {
	
	//	set the actual size and not that bs in bits
	blockSize /= 8;

	std::vector <uint8_t> hash;
		hash.resize(blockSize);

	if (!data.size()) {
		hashBlock(hash.data(), blockSize);
		return hash;
	}

	const size_t padding = (data.size() < blockSize) ? (blockSize - data.size()) : (data.size() % blockSize);
	if (padding) data.resize(data.size() + padding, 0);

	for (size_t i = 0; i < data.size(); i+= blockSize) {
		hashBlock(data.data() + i, blockSize);
		xorTo(hash.data(), data.data() + i, blockSize);
	}
	
	return hash;
}
