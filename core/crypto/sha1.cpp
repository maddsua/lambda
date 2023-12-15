/*

	SHA-1

	Original code by Brad Conte (brad AT bradconte.com) from https://github.com/B-Con/crypto-algorithms
	No license provided, assuming as a public domain

*/

#include <memory.h>

#include "./crypto.hpp"

using namespace Lambda;
using namespace Lambda::Crypto;

struct SHA1_CTX {
	uint8_t data[64];
	uint32_t datalen;
	size_t bitlen;
	uint32_t state[5];
	uint32_t k[4];
};

#define SHA1_ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

void SHA1::sha1_transform() {

	auto ctx = (SHA1_CTX*)this->hashctx;

	uint32_t block[80];
	for (unsigned int i = 0, j = 0; i < 80; ++i) {
		if (i < 16) {
			block[i] = (ctx->data[j] << 24) + (ctx->data[j + 1] << 16) + (ctx->data[j + 2] << 8) + (ctx->data[j + 3]);
			j += 4;

		} else {
			block[i] = (block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16]);
			block[i] = (block[i] << 1) | (block[i] >> 31);
		}
	}

	uint32_t shift[5], t0;
	for (unsigned int i = 0; i < 5; i++)
		shift[i] = ctx->state[i];

	for (unsigned int i = 0; i < 80; ++i) {

		if (i < 20) 
			t0 = SHA1_ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (~shift[1] & shift[3])) + shift[4] + ctx->k[0] + block[i];
		else if (i < 40) 
			t0 = SHA1_ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + ctx->k[1] + block[i];
		else if (i < 60) 
			t0 = SHA1_ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (shift[1] & shift[3]) ^ (shift[2] & shift[3])) + shift[4] + ctx->k[2] + block[i];
		else if (i < 80) 
			t0 = SHA1_ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + ctx->k[3] + block[i];

		shift[4] = shift[3];
		shift[3] = shift[2];
		shift[2] = SHA1_ROTLEFT(shift[1], 30);
		shift[1] = shift[0];
		shift[0] = t0;
	}
	
	for (unsigned int i = 0; i < 5; i++)
		ctx->state[i] += shift[i];
}

SHA1::SHA1() {
	this->hashctx = new SHA1_CTX;
	this->reset();
}

SHA1::~SHA1() {
	delete (SHA1_CTX*)this->hashctx;
}

void SHA1::reset() {

	auto ctx = (SHA1_CTX*)this->hashctx;
	memset(this->hashctx, 0, sizeof(SHA1_CTX));

	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xc3d2e1f0;
	ctx->k[0] = 0x5a827999;
	ctx->k[1] = 0x6ed9eba1;
	ctx->k[2] = 0x8f1bbcdc;
	ctx->k[3] = 0xca62c1d6;
}

void SHA1::update(const std::vector<uint8_t>& data) {
	auto ctx = (SHA1_CTX*)this->hashctx;
	for (size_t i = 0; i < data.size(); ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			sha1_transform();
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

std::array <uint8_t, SHA1::BlockSize> SHA1::digest() {

	auto ctx = (SHA1_CTX*)this->hashctx;
	std::array <uint8_t, SHA1::BlockSize> hash;
	uint32_t i = ctx->datalen;

	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56) ctx->data[i++] = 0x00;

	} else {
		ctx->data[i++] = 0x80;
		while (i < 64) ctx->data[i++] = 0x00;
		sha1_transform();
		memset(ctx->data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[63] = ctx->bitlen;
	ctx->data[62] = ctx->bitlen >> 8;
	ctx->data[61] = ctx->bitlen >> 16;
	ctx->data[60] = ctx->bitlen >> 24;
	ctx->data[59] = ctx->bitlen >> 32;
	ctx->data[58] = ctx->bitlen >> 40;
	ctx->data[57] = ctx->bitlen >> 48;
	ctx->data[56] = ctx->bitlen >> 56;
	sha1_transform();

	// Since this implementation uses little endian byte ordering and MD uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
	}

	return hash;
}
