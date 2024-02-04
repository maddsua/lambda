
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

namespace Lambda {

	namespace Errors {
		int32_t getApiError() noexcept;
		std::string formatMessage(int32_t errorCode) noexcept;
	};

	class APIError : public std::exception {
		private:
			int32_t m_code = 0;
			std::string m_text;

		public:
			APIError();
			APIError(int32_t code);
			APIError(int32_t code, const std::string& commentText);
			APIError(const std::string& commentText);
			APIError(const APIError& other);
			const char* what() const noexcept override;
			const std::string& message() const noexcept;
			int32_t code() const noexcept;
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

	namespace Console {

			struct LogEntry {
				LogEntry(const std::string& thing);
				LogEntry(const char* thing);
				LogEntry(bool thing);
				LogEntry(char thing);
				LogEntry(unsigned char thing);
				LogEntry(short thing);
				LogEntry(unsigned short thing);
				LogEntry(int thing);
				LogEntry(unsigned int thing);
				LogEntry(float thing);
				LogEntry(double thing);
				LogEntry(long thing);
				LogEntry(unsigned long thing);
				LogEntry(long long thing);
				LogEntry(unsigned long long thing);
				LogEntry(long double thing);

				std::string value;
			};

		class ConsoleImpl {
			private:
				std::string serializeEntries(const std::initializer_list<LogEntry>& list) const noexcept;
				std::mutex m_write_lock;

			public:
				void log(std::initializer_list<LogEntry> list) noexcept;
				void error(std::initializer_list<LogEntry> list) noexcept;
				void warn(std::initializer_list<LogEntry> list) noexcept;
		};
	};

	extern Console::ConsoleImpl console;

};

#endif
