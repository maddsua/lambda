
#ifndef __LIB_MADDSUA_LAMBDA_CORE_UTILS_BYTESWAP__
#define __LIB_MADDSUA_LAMBDA_CORE_UTILS_BYTESWAP__

	#include <cstdint>

	inline uint16_t swapByteOrder(uint16_t val) {
		return (val << 8) | (val >> 8 );
	}

	inline int16_t swapByteOrder(int16_t val) {
		return (val << 8) | ((val >> 8) & 0xFF);
	}

	inline uint32_t swapByteOrder(uint32_t val) {
		val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
		return (val << 16) | (val >> 16);
	}

	inline int32_t swapByteOrder(int32_t val) {
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
		return (val << 16) | ((val >> 16) & 0xFFFF);
	}

	inline int64_t swapByteOrder(int64_t val) {
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
		return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
	}

	inline uint64_t swapByteOrder(uint64_t val) {
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
		return (val << 32) | (val >> 32);
	}

	#ifdef _WIN32
		#define normalizeByteOrder(value) (value)
	#else
		#define normalizeByteOrder(value) (swapByteOrder(value))
	#endif

#endif
