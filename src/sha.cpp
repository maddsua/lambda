/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include "../include/lambda/crypto.hpp"


//	Refactored for maddsua/lambda on Feb 04 2023
//	Borrowed some code from the smarter guys

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

std::array <uint8_t, SHA256_BLOCK_SIZE> lambda::sha256Hash(std::vector<uint8_t> data) {

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


/*

	SHA-512
	
	Original source code by Marek Ulwa??ski from https://github.com/ulwanski/sha512
	No license provided, assuming as a public domain
	
*/

#define SHA512_SHFR(x, n)    (x >> n)
#define SHA512_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA512_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA512_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA512_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA512_F1(x) (SHA512_ROTR(x, 28) ^ SHA512_ROTR(x, 34) ^ SHA512_ROTR(x, 39))
#define SHA512_F2(x) (SHA512_ROTR(x, 14) ^ SHA512_ROTR(x, 18) ^ SHA512_ROTR(x, 41))
#define SHA512_F3(x) (SHA512_ROTR(x,  1) ^ SHA512_ROTR(x,  8) ^ SHA512_SHFR(x,  7))
#define SHA512_F4(x) (SHA512_ROTR(x, 19) ^ SHA512_ROTR(x, 61) ^ SHA512_SHFR(x,  6))
#define SHA512_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8_t) ((x)      );       \
    *((str) + 2) = (uint8_t) ((x) >>  8);       \
    *((str) + 1) = (uint8_t) ((x) >> 16);       \
    *((str) + 0) = (uint8_t) ((x) >> 24);       \
}
#define SHA512_UNPACK64(x, str)                 \
{                                             \
    *((str) + 7) = (uint8_t) ((x)      );       \
    *((str) + 6) = (uint8_t) ((x) >>  8);       \
    *((str) + 5) = (uint8_t) ((x) >> 16);       \
    *((str) + 4) = (uint8_t) ((x) >> 24);       \
    *((str) + 3) = (uint8_t) ((x) >> 32);       \
    *((str) + 2) = (uint8_t) ((x) >> 40);       \
    *((str) + 1) = (uint8_t) ((x) >> 48);       \
    *((str) + 0) = (uint8_t) ((x) >> 56);       \
}
#define SHA512_PACK64(str, x)                   \
{                                             \
    *(x) =   ((uint64_t) *((str) + 7)      )    \
           | ((uint64_t) *((str) + 6) <<  8)    \
           | ((uint64_t) *((str) + 5) << 16)    \
           | ((uint64_t) *((str) + 4) << 24)    \
           | ((uint64_t) *((str) + 3) << 32)    \
           | ((uint64_t) *((str) + 2) << 40)    \
           | ((uint64_t) *((str) + 1) << 48)    \
           | ((uint64_t) *((str) + 0) << 56);   \
}

const uint64_t sha512_k[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
	0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
	0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
	0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
	0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
	0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
	0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
	0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
	0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
	0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
	0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
	0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
	0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
	0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
	0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
	0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
	0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
	0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
	0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
	0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
	0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

struct SHA512CTX {
	size_t length;
	size_t blockLen;
	uint8_t block[2 * SHA512_BLOCK_SIZE];
	size_t state[8];
};

void sha512_Transform(SHA512CTX* ctx, const uint8_t* message, size_t block_nb) {

	uint64_t block[80];
	uint64_t shift[8];

	const uint8_t* sub_block;

	for (size_t i = 0, j = 0; i < block_nb; i++) {

		sub_block = message + (i << 7);

		for (j = 0; j < 16; j++)
			SHA512_PACK64(&sub_block[j << 3], &block[j]);

		for (j = 16; j < 80; j++)
			block[j] = SHA512_F4(block[j - 2]) + block[j - 7] + SHA512_F3(block[j - 15]) + block[j - 16];

		for (j = 0; j < 8; j++)
			shift[j] = ctx->state[j];

		for (j = 0; j < 80; j++) {
			
			uint64_t t1 = shift[7] + SHA512_F2(shift[4]) + SHA512_CH(shift[4], shift[5], shift[6]) + sha512_k[j] + block[j];
			uint64_t t2 = SHA512_F1(shift[0]) + SHA512_MAJ(shift[0], shift[1], shift[2]);

			shift[7] = shift[6];
			shift[6] = shift[5];
			shift[5] = shift[4];
			shift[4] = shift[3] + t1;
			shift[3] = shift[2];
			shift[2] = shift[1];
			shift[1] = shift[0];
			shift[0] = t1 + t2;
		}

		for (j = 0; j < 8; j++)
			ctx->state[j] += shift[j];
	}

	return;
}


std::array <uint8_t, SHA512_HASH_SIZE> lambda::sha512Hash(std::vector<uint8_t> data) {

	std::array <uint8_t, SHA512_HASH_SIZE> hash;

	//	sha-512 init stage
	SHA512CTX ctx;
	{
		ctx.state[0] = 0x6a09e667f3bcc908ULL;
		ctx.state[1] = 0xbb67ae8584caa73bULL;
		ctx.state[2] = 0x3c6ef372fe94f82bULL;
		ctx.state[3] = 0xa54ff53a5f1d36f1ULL;
		ctx.state[4] = 0x510e527fade682d1ULL;
		ctx.state[5] = 0x9b05688c2b3e6c1fULL;
		ctx.state[6] = 0x1f83d9abfb41bd6bULL; 
		ctx.state[7] = 0x5be0cd19137e2179ULL;
		ctx.blockLen = 0;
		ctx.length = 0;
	}

	// sha-512 message update stage
	{
		size_t tmp_len = SHA512_BLOCK_SIZE - ctx.blockLen;
		size_t rem_len = (data.size() < tmp_len) ? data.size() : tmp_len;
		size_t new_len = data.size() - rem_len;
		size_t block_nb = new_len / SHA512_BLOCK_SIZE;

		memcpy(&ctx.block[ctx.blockLen], data.data(), rem_len);

		if ((ctx.blockLen + data.size() >= SHA512_BLOCK_SIZE)) {
			const uint8_t* shifted_message = data.data() + rem_len;

			sha512_Transform(&ctx, ctx.block, 1);
			sha512_Transform(&ctx, shifted_message, block_nb);

			rem_len = new_len % SHA512_BLOCK_SIZE;

			memcpy(ctx.block, &shifted_message[block_nb << 7], rem_len);

			ctx.blockLen = rem_len;
			ctx.length += (block_nb + 1) << 7;

		} else ctx.blockLen += data.size();
	}

	//	sha-512 final stage
	{
		size_t block_nb = 1 + ((SHA512_BLOCK_SIZE - 17) < (ctx.blockLen % SHA512_BLOCK_SIZE));
		size_t len_b = (ctx.length + ctx.blockLen) << 3;
		size_t pm_len = block_nb << 7;

		memset(ctx.block + ctx.blockLen, 0, pm_len - ctx.blockLen);
		ctx.block[ctx.blockLen] = 128;

		SHA512_UNPACK32(len_b, ctx.block + pm_len - 4);
		sha512_Transform(&ctx, ctx.block, block_nb);

		for (size_t i = 0 ; i < 8; i++) {
			SHA512_UNPACK64(ctx.state[i], &hash[i << 3]);
		}
	}

	return hash;
}




/*

	SHA-1

	Original code by Brad Conte (brad AT bradconte.com) from https://github.com/B-Con/crypto-algorithms
	No license provided, assuming as a public domain

*/

#define SHA1_ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

struct SHA1_CTX {
	uint8_t data[64];
	uint32_t datalen;
	size_t bitlen;
	uint32_t state[5];
	uint32_t k[4];
};

void sha1_transform(SHA1_CTX *ctx, const uint8_t* data) {

	uint32_t block[80];
	for (unsigned int i = 0, j = 0; i < 80; ++i) {
		if (i < 16) {
			block[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
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


std::array <uint8_t, SHA1_BLOCK_SIZE> lambda::sha1Hash(std::vector<uint8_t> data) {

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