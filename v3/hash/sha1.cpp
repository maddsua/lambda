#include "./hash.hpp"

using namespace Lambda::Hash;

//	Original code by Brad Conte (brad AT bradconte.com),
//	 from https://github.com/B-Con/crypto-algorithms

class Sha1 {
	private:
		uint8_t data[64] = {0};
		uint32_t datalen;
		size_t bitlen;
		uint32_t state[5];
		uint32_t k[4];

		void m_transform() {

			#define SHA1_ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

			uint32_t block[80];
			for (uint32_t i = 0, j = 0; i < 80; ++i) {

				if (i >= 16) {
					block[i] = (block[i - 3] ^ block[i - 8] ^ block[i - 14] ^ block[i - 16]);
					block[i] = (block[i] << 1) | (block[i] >> 31);
					continue;
				}

				block[i] = (this->data[j] << 24) + (this->data[j + 1] << 16) + (this->data[j + 2] << 8) + (this->data[j + 3]);
				j += 4;
			}

			uint32_t shift[5], t0;
			for (uint32_t i = 0; i < 5; i++) {
				shift[i] = this->state[i];
			}

			for (uint32_t i = 0; i < 80; ++i) {

				if (i < 20)  {
					t0 = SHA1_ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (~shift[1] & shift[3])) + shift[4] + this->k[0] + block[i];
				} else if (i < 40) {
					t0 = SHA1_ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + this->k[1] + block[i];
				} else if (i < 60) {
					t0 = SHA1_ROTLEFT(shift[0], 5) + ((shift[1] & shift[2]) ^ (shift[1] & shift[3]) ^ (shift[2] & shift[3])) + shift[4] + this->k[2] + block[i];
				} else if (i < 80) {
					t0 = SHA1_ROTLEFT(shift[0], 5) + (shift[1] ^ shift[2] ^ shift[3]) + shift[4] + this->k[3] + block[i];
				}

				shift[4] = shift[3];
				shift[3] = shift[2];
				shift[2] = SHA1_ROTLEFT(shift[1], 30);
				shift[1] = shift[0];
				shift[0] = t0;
			}

			for (unsigned int i = 0; i < 5; i++) {
				this->state[i] += shift[i];
			}
		}

	public:
		static const size_t BlockSize = 20;

		Sha1() {
			this->datalen = 0;
			this->bitlen = 0;
			this->state[0] = 0x67452301;
			this->state[1] = 0xEFCDAB89;
			this->state[2] = 0x98BADCFE;
			this->state[3] = 0x10325476;
			this->state[4] = 0xc3d2e1f0;
			this->k[0] = 0x5a827999;
			this->k[1] = 0x6ed9eba1;
			this->k[2] = 0x8f1bbcdc;
			this->k[3] = 0xca62c1d6;
		}

		Sha1& update(const std::vector<uint8_t>& data) {
			
			for (size_t idx = 0; idx < data.size(); ++idx) {

				this->data[this->datalen] = data[idx];
				this->datalen++;

				if (this->datalen == 64) {
					this->m_transform();
					this->bitlen += 512;
					this->datalen = 0;
				}
			}

			return *this;
		}

		std::vector<uint8_t> digest() {

			std::vector<uint8_t> hash;
			hash.resize(Sha1::BlockSize);

			uint32_t i = this->datalen;

			// Pad whatever data is left in the buffer.
			if (this->datalen < 56) {

				this->data[i++] = 0x80;

				while (i < 56) {
					this->data[i++] = 0x00;
				}

			} else {

				this->data[i++] = 0x80;

				while (i < 64) {
					this->data[i++] = 0x00;
				}
	
				this->m_transform();

				for (uint32_t i = 0; i < 56; i++) {
					this->data[i] = 0;
				}
			}

			// Append to the padding the total message's length in bits and transform.
			this->bitlen += this->datalen * 8;
			this->data[63] = this->bitlen;
			this->data[62] = this->bitlen >> 8;
			this->data[61] = this->bitlen >> 16;
			this->data[60] = this->bitlen >> 24;
			this->data[59] = this->bitlen >> 32;
			this->data[58] = this->bitlen >> 40;
			this->data[57] = this->bitlen >> 48;
			this->data[56] = this->bitlen >> 56;
			this->m_transform();

			// Since this implementation uses little endian byte ordering and MD uses big endian,
			// reverse all the bytes when copying the final state to the output hash.
			for (i = 0; i < 4; ++i) {
				hash[i]      = (this->state[0] >> (24 - i * 8)) & 0x000000ff;
				hash[i + 4]  = (this->state[1] >> (24 - i * 8)) & 0x000000ff;
				hash[i + 8]  = (this->state[2] >> (24 - i * 8)) & 0x000000ff;
				hash[i + 12] = (this->state[3] >> (24 - i * 8)) & 0x000000ff;
				hash[i + 16] = (this->state[4] >> (24 - i * 8)) & 0x000000ff;
			}

			return hash;

		}
};

std::vector<uint8_t> SHA1::text(const std::string& data) {
	return Sha1().update(std::vector<uint8_t>(data.begin(), data.end())).digest();
}
