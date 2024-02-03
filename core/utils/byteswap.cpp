#include "./utils.hpp"

using namespace Lambda;
using namespace Lambda::Utils;


#define swap_uint16(data)	(((data) << 8) | ((data) >> 8))
#define swap_int16(data)	((((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00))

#define swap_uint32(data)	\
	(((data) >> 24) | (((data) >>  8) & 0x0000FF00) | \
	(((data) <<  8) & 0x00FF0000) | ((data) << 24))

#define swap_int32(data)	\
	((((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
	(((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000))

#define swap_uint64(data)	\
	(((data) >> 56) | (((data) >> 40) & 0x000000000000FF00) | \
	(((data) >> 24) & 0x0000000000FF0000) | (((data) >>  8) & 0x00000000FF000000) | \
	(((data) <<  8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
	(((data) << 40) & 0x00FF000000000000) | ((data) << 56))

#define swap_int64(data)	\
	((((data) >> 56) & 0x00000000000000FF) | (((data) >> 40) & 0x000000000000FF00) | \
	(((data) >> 24) & 0x0000000000FF0000) | (((data) >>  8) & 0x00000000FF000000) | \
	(((data) <<  8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
	(((data) << 40) & 0x00FF000000000000) | (((data) << 56) & 0xFF00000000000000))


int16_t Bits::netwnormx(int16_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_int16(val);
	#else
		return val;
	#endif
}

uint16_t Bits::netwnormx(uint16_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_uint16(val);
	#else
		return val;
	#endif
}

int32_t Bits::netwnormx(int32_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_int32(val);
	#else
		return val;
	#endif
}

uint32_t Bits::netwnormx(uint32_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_uint32(val);
	#else
		return val;
	#endif
}

int64_t Bits::netwnormx(int64_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_int64(val);
	#else
		return val;
	#endif
}

uint64_t Bits::netwnormx(uint64_t val) {
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return swap_uint64(val);
	#else
		return val;
	#endif
}


int16_t Bits::storenormx(int16_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_int16(val);
	#else
		return val;
	#endif
}

uint16_t Bits::storenormx(uint16_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_uint16(val);
	#else
		return val;
	#endif
}

int32_t Bits::storenormx(int32_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_int32(val);
	#else
		return val;
	#endif
}

uint32_t Bits::storenormx(uint32_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_uint32(val);
	#else
		return val;
	#endif
}

int64_t Bits::storenormx(int64_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_int64(val);
	#else
		return val;
	#endif
}

uint64_t Bits::storenormx(uint64_t val) {
	#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		return swap_uint64(val);
	#else
		return val;
	#endif
}
