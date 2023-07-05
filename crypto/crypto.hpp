#ifndef __LAMBDA_CRYPTO__
#define __LAMBDA_CRYPTO__

#include <stdint.h>
#include <string>
#include <vector>
#include <array>
#include <memory.h>

namespace Lambda::Crypto {

	enum Constants {
		SHA1_BLOCK_SIZE = 20,
	};

	class SHA1 {
		private:
			void* hashctx = nullptr;
			void sha1_transform();

		public:
			SHA1();
			~SHA1();
			void reset();
			void update(const std::vector<uint8_t>& data);
			std::array <uint8_t, Crypto::SHA1_BLOCK_SIZE> digest();
	};

	std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
	std::vector <uint8_t> randomStream(const size_t length);
}

#endif
