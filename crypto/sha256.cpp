#include "crypto.hpp"

/*

	SHA-256

	Original code by Brad Conte (brad AT bradconte.com) from https://github.com/B-Con/crypto-algorithms
	No license provided, assuming as a public domain

*/

#define SHA256_ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define SHA256_ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define SHA256_CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x) (SHA256_ROTRIGHT(x,2) ^ SHA256_ROTRIGHT(x,13) ^ SHA256_ROTRIGHT(x,22))
#define SHA256_EP1(x) (SHA256_ROTRIGHT(x,6) ^ SHA256_ROTRIGHT(x,11) ^ SHA256_ROTRIGHT(x,25))
#define SHA256_SIG0(x) (SHA256_ROTRIGHT(x,7) ^ SHA256_ROTRIGHT(x,18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_ROTRIGHT(x,17) ^ SHA256_ROTRIGHT(x,19) ^ ((x) >> 10))


static const uint32_t sha256_k[64] = {
	0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
	0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
	0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
	0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
	0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
	0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
	0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
	0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

struct SHA256CTX {
	uint8_t data[64];
	size_t datalen;
	size_t bitlen;
	uint32_t state[8];
};

void sha256_Transform(SHA256CTX* ctx, const uint8_t* data) {

	uint32_t block[64];
	for (unsigned int i = 0, j = 0; i < 64; ++i) {
		if (i < 16) {
			block[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
			j += 4;

		} else {
			block[i] = SHA256_SIG1(block[i - 2]) + block[i - 7] + SHA256_SIG0(block[i - 15]) + block[i - 16];
		}
	}

	uint32_t shifts[8];
	for (size_t i = 0; i < 8; i++)
		shifts[i] = ctx->state[i];
	
	uint32_t t1, t2;
	for (unsigned int i = 0; i < 64; ++i) {
		t1 = shifts[7] + SHA256_EP1(shifts[4]) + SHA256_CH(shifts[4], shifts[5], shifts[6]) + sha256_k[i] + block[i];
		t2 = SHA256_EP0(shifts[0]) + SHA256_MAJ(shifts[0], shifts[1], shifts[2]);
		shifts[7] = shifts[6];
		shifts[6] = shifts[5];
		shifts[5] = shifts[4];
		shifts[4] = shifts[3] + t1;
		shifts[3] = shifts[2];
		shifts[2] = shifts[1];
		shifts[1] = shifts[0];
		shifts[0] = t1 + t2;
	}

	for (size_t i = 0; i < 8; i++)
		ctx->state[i] += shifts[i];

	return;
}

std::array <uint8_t, SHA256_BLOCK_SIZE> Lambda::Crypto::sha256Hash(std::vector<uint8_t> data) {

	std::array <uint8_t, SHA256_BLOCK_SIZE> result;

	//	sha-256 init stage
	SHA256CTX ctx;
	{
		ctx.datalen = 0;
		ctx.bitlen = 0;
		ctx.state[0] = 0x6a09e667;
		ctx.state[1] = 0xbb67ae85;
		ctx.state[2] = 0x3c6ef372;
		ctx.state[3] = 0xa54ff53a;
		ctx.state[4] = 0x510e527f;
		ctx.state[5] = 0x9b05688c;
		ctx.state[6] = 0x1f83d9ab;
		ctx.state[7] = 0x5be0cd19;
	}

	//	sha-256 update stage
	for (size_t i = 0; i < data.size(); ++i) {

		ctx.data[ctx.datalen] = data[i];
		ctx.datalen++;

		if (ctx.datalen == 64) {
			sha256_Transform(&ctx, ctx.data);
			ctx.bitlen += 512;
			ctx.datalen = 0;
		}
	}

	//	sha-256 final stage
	{
		size_t i = ctx.datalen;

		// Pad whatever data is left in the buffer.
		if (ctx.datalen < 56) {
			
			ctx.data[i++] = 0x80;
			while (i < 56) ctx.data[i++] = 0x00;

		} else {

			ctx.data[i++] = 0x80;
			while (i < 64) ctx.data[i++] = 0x00;

			sha256_Transform(&ctx, ctx.data);
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
		
		sha256_Transform(&ctx, ctx.data);

		// Since this implementation uses little endian byte ordering and SHA uses big endian,
		// reverse all the bytes when copying the final state to the output hash.
		for (i = 0; i < 4; ++i) {
			result[i]      = (ctx.state[0] >> (24 - i * 8)) & 0x000000ff;
			result[i + 4]  = (ctx.state[1] >> (24 - i * 8)) & 0x000000ff;
			result[i + 8]  = (ctx.state[2] >> (24 - i * 8)) & 0x000000ff;
			result[i + 12] = (ctx.state[3] >> (24 - i * 8)) & 0x000000ff;
			result[i + 16] = (ctx.state[4] >> (24 - i * 8)) & 0x000000ff;
			result[i + 20] = (ctx.state[5] >> (24 - i * 8)) & 0x000000ff;
			result[i + 24] = (ctx.state[6] >> (24 - i * 8)) & 0x000000ff;
			result[i + 28] = (ctx.state[7] >> (24 - i * 8)) & 0x000000ff;
		}
	}

	return result;
}