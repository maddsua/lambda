#ifndef __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__
#define __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__

#include <string>
#include <vector>
#include <array>

namespace Lambda::Crypto {

	std::vector<uint8_t> getRandomValues(size_t length);
	void getRandomValues(std::vector<uint8_t>& buffer);
	std::string randomUUID();
	std::string randomID(size_t length);
	std::string randomString(size_t length);

	class SHA1 {
		private:
			void* hashctx = nullptr;
			void sha1_transform();

		public:
			static const size_t BlockSize = 20;

			SHA1();
			~SHA1();
			void reset();
			void update(const std::vector<uint8_t>& data);
			std::array <uint8_t, SHA1::BlockSize> digest();
	};

}

#endif
