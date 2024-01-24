#ifndef __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__
#define __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__

#include <string>
#include <vector>

namespace Lambda::Crypto {

	class SHA1 {
		private:
			void* hashctx = nullptr;
			void sha1_transform();
			void update(const uint8_t* data, size_t dataSize);

		public:
			static const size_t BlockSize = 20;

			SHA1();
			~SHA1();
			SHA1& reset();
			SHA1& update(const std::vector<uint8_t>& buffer);
			SHA1& update(const std::string& text);
			std::vector <uint8_t> digest();
	};

}

#endif
