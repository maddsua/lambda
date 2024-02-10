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
			std::vector<uint8_t> digest();
	};

	class ShortID {
		private:

			union {
				uint32_t u32 = 0;
				uint8_t buff[sizeof(uint32_t)];
			} m_id;

			std::string m_str;

			void m_serialize() noexcept;

		public:
			ShortID();
			ShortID(uint32_t init);

			ShortID(const ShortID& other) noexcept;
			ShortID(ShortID&& other) noexcept;

			ShortID& operator=(const ShortID& other) noexcept;

			void update() noexcept;
			void update(uint32_t value) noexcept;

			uint32_t id() const noexcept;
			const std::string& toString() const noexcept;
	};
}

#endif
