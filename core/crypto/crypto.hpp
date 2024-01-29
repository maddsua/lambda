#ifndef __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__
#define __LIB_MADDSUA_LAMBDA_CORE_CRYPTO__

#include <string>
#include <vector>
#include <array>

namespace Lambda::Crypto {

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

	class ShortID {
		private:
			union {
				uint32_t numeric = 0;
				char buffer[sizeof(uint32_t)];
			} m_id;

		public:
			ShortID();
			ShortID(uint32_t init);
			std::string toString() const;
	};
}

#endif
