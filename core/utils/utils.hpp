
/*
	Lambda defaults to little-endian byte order.
	Just for the sake of simplicity, as most systems
	that it's intended to run on are x86-64
*/

#ifndef __LIB_MADDSUA_LAMBDA_CORE_UTILS__
#define __LIB_MADDSUA_LAMBDA_CORE_UTILS__

#include <cstdint>

namespace Lambda::Utils {

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
};

#endif