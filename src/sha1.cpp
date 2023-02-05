#include "../include/maddsua/crypto.hpp"

//	Refactored for maddsua/lambda on Feb 04 2023
//	Original code by Brad Conte (brad AT bradconte.com) from https://github.com/B-Con/crypto-algorithms


#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))


struct SHA1_CTX {
	uint8_t data[64];
	uint32_t datalen;
	size_t bitlen;
	uint32_t state[5];
	uint32_t k[4];
};


void sha1_transform(SHA1_CTX *ctx, const uint8_t* data) {

	uint32_t block[80];
	for (uint16_t i = 0, j = 0; i < 80; ++i) {
		if (i < 16) {
			block[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
			j += 4;

		} else {
			block[i] = (block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16]);
			block[i] = (block[i] << 1) | (block[i] >> 31);
		}
	}

	uint32_t shift[5], t0;
	for (uint16_t i = 0; i < 5; i++)
		shift[i] = ctx->state[i];

	for (uint16_t i = 0; i < 80; ++i) {

		if (i < 20) 
			t0 = ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (~shift[1] & shift[3])) + shift[4] + ctx->k[0] + block[i];
		else if (i < 40) 
			t0 = ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + ctx->k[1] + block[i];
		else if (i < 60) 
			t0 = ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (shift[1] & shift[3]) ^ (shift[2] & shift[3])) + shift[4] + ctx->k[2] + block[i];
		else if (i < 80) 
			t0 = ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + ctx->k[3] + block[i];

		shift[4] = shift[3];
		shift[3] = shift[2];
		shift[2] = ROTLEFT(shift[1], 30);
		shift[1] = shift[0];
		shift[0] = t0;
	}
	
	for (uint16_t i = 0; i < 5; i++)
		ctx->state[i] += shift[i];
}


std::array <uint8_t, SHA1_BLOCK_SIZE> maddsua::sha1Hash(std::vector<uint8_t> data) {

	std::array <uint8_t, SHA1_BLOCK_SIZE> result;

	//	sha-1 init stage
	SHA1_CTX ctx;
	{
		ctx.datalen = 0;
		ctx.bitlen = 0;
		ctx.state[0] = 0x67452301;
		ctx.state[1] = 0xEFCDAB89;
		ctx.state[2] = 0x98BADCFE;
		ctx.state[3] = 0x10325476;
		ctx.state[4] = 0xc3d2e1f0;
		ctx.k[0] = 0x5a827999;
		ctx.k[1] = 0x6ed9eba1;
		ctx.k[2] = 0x8f1bbcdc;
		ctx.k[3] = 0xca62c1d6;
	}

	//	sha-1 update stage
	for (size_t i = 0; i < data.size(); ++i) {
		ctx.data[ctx.datalen] = data[i];
		ctx.datalen++;
		if (ctx.datalen == 64) {
			sha1_transform(&ctx, ctx.data);
			ctx.bitlen += 512;
			ctx.datalen = 0;
		}
	}

	//	sha-1 final stage
	{
		uint32_t i = ctx.datalen;

		// Pad whatever data is left in the buffer.
		if (ctx.datalen < 56) {
			ctx.data[i++] = 0x80;
			while (i < 56) ctx.data[i++] = 0x00;

		} else {
			ctx.data[i++] = 0x80;
			while (i < 64) ctx.data[i++] = 0x00;
			sha1_transform(&ctx, ctx.data);
			memset(ctx.data, 0, 56);
		}

		// Append to the padding the total message's length in bits and transform.
		ctx.bitlen += ctx.datalen * 8;
		ctx.data[63] = ctx.bitlen;
		ctx.data[62] = ctx.bitlen >> 8;
		ctx.data[61] = ctx.bitlen >> 16;
		ctx.data[60] = ctx.bitlen >> 24;
		ctx.data[59] = ctx.bitlen >> 32;
		ctx.data[58] = ctx.bitlen >> 40;
		ctx.data[57] = ctx.bitlen >> 48;
		ctx.data[56] = ctx.bitlen >> 56;
		sha1_transform(&ctx, ctx.data);

		// Since this implementation uses little endian byte ordering and MD uses big endian,
		// reverse all the bytes when copying the final state to the output hash.
		for (i = 0; i < 4; ++i) {
			result[i]      = (ctx.state[0] >> (24 - i * 8)) & 0x000000ff;
			result[i + 4]  = (ctx.state[1] >> (24 - i * 8)) & 0x000000ff;
			result[i + 8]  = (ctx.state[2] >> (24 - i * 8)) & 0x000000ff;
			result[i + 12] = (ctx.state[3] >> (24 - i * 8)) & 0x000000ff;
			result[i + 16] = (ctx.state[4] >> (24 - i * 8)) & 0x000000ff;
		}
	}

	return result;
}