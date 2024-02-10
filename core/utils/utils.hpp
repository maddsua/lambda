
/*
	Lambda defaults to little-endian byte order.
	Just for the sake of simplicity, as most systems
	that it's intended to run on are x86-64
*/

#ifndef __LIB_MADDSUA_LAMBDA_CORE_UTILS__
#define __LIB_MADDSUA_LAMBDA_CORE_UTILS__

#include <cstdint>
#include <stdexcept>
#include <string>
#include <mutex>
#include <optional>

namespace Lambda {

	class OS_Error {
		int32_t m_code = 0;

		OS_Error();
		OS_Error(int32_t code);
		int32_t code() const noexcept;
		std::string toString() const noexcept;
	};

	class Error : public std::exception {
		protected:
			std::string m_text;

		public:
			Error(const std::string& message) : m_text(message) {}
			Error(const char* message) : m_text(message) {}
			Error(const std::exception& init) : m_text(init.what()) {}

			const char* what() const noexcept override { return this->m_text.c_str(); }
			const std::string& message() const noexcept { return this->m_text; }
	};

	namespace Bits {

		/**
		 * Normalize host/network byte order
		 * Network byte order is big endian.
		 * These functions swap byte order on little-endian systems and leave it intact otherwise
		 * Is an overloaded function for all int/uint 16-64 for both way conversions
		*/
		int16_t netwnormx(int16_t val);
		uint16_t netwnormx(uint16_t val);
		int32_t netwnormx(int32_t val);
		uint32_t netwnormx(uint32_t val);
		int64_t netwnormx(int64_t val);
		uint64_t netwnormx(uint64_t val);

		/**
		 * Normalize "storage" byte order
		 * "Storage" byte order is little endian.
		 * Swaps endianess on big-endian systems and leaves it intact otherwise
		 * Is an overloaded function for all int/uint 16-64 for both way conversions
		*/
		int16_t storenormx(int16_t val);
		uint16_t storenormx(uint16_t val);
		int32_t storenormx(int32_t val);
		uint32_t storenormx(uint32_t val);
		int64_t storenormx(int64_t val);
		uint64_t storenormx(uint64_t val);
	};

	namespace Literals {

		constexpr size_t operator""_GB(unsigned long long size) {
			return 1024 * 1024 * 1024 * size;
		}

		constexpr size_t operator""_MB(unsigned long long size) {
			return 1024 * 1024 * size;
		}

		constexpr size_t operator""_KB(unsigned long long size) {
			return 1024 * size;
		}
	};

	namespace SyncOut {

		struct MgsOverload {
			MgsOverload() = default;
			MgsOverload(const std::string& thing);
			MgsOverload(const char* thing);
			MgsOverload(bool thing);
			MgsOverload(char thing);
			MgsOverload(unsigned char thing);
			MgsOverload(short thing);
			MgsOverload(unsigned short thing);
			MgsOverload(int thing);
			MgsOverload(unsigned int thing);
			MgsOverload(float thing);
			MgsOverload(double thing);
			MgsOverload(long thing);
			MgsOverload(unsigned long thing);
			MgsOverload(long long thing);
			MgsOverload(unsigned long long thing);
			MgsOverload(long double thing);

			std::optional<std::string> valueOpt;
		};

		class WrapperImpl {
			private:
				std::optional<std::string> serializeEntries(const std::initializer_list<MgsOverload>& list) const noexcept;
				std::mutex m_write_lock;

			public:
				/**
				 * Print log items to stdout
				*/
				void log(MgsOverload item) noexcept;
				void log(std::initializer_list<MgsOverload> list) noexcept;
				/**
				 * Print log items to stderr
				*/
				void error(MgsOverload item) noexcept;
				void error(std::initializer_list<MgsOverload> list) noexcept;
		};
	};

	/**
	 * Thread-safe stdout and stderr in a single pagages.
	 * Type overloads included. Yummy!
	*/
	extern SyncOut::WrapperImpl syncout;

};

#endif
