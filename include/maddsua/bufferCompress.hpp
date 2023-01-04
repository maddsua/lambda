/*

	This wrapper was created so compression just works.

	Requirements:
		zlib: https://github.com/madler/zlib
	
*/


#include <zlib.h>

#include <stdint.h>
#include <vector>

#ifndef _maddsua_compression_wrapper
#define _maddsua_compression_wrapper

#define ZLIB_MEXP_HEADER_Z		(8)
#define ZLIB_MEXP_HEADER_GZ		(26)
#define ZLIB_MEXP_DECOMP_GZ		(16)
#define ZLIB_MEXP_DECOM_AUTO	(32)
#define ZLIB_MEXP_RAW			(-15)

#define ZLIB_MEXP_MEM_MAX		(9)
#define ZLIB_MEXP_CHUNK			(262144)	//	256k

namespace maddsuaCompress {

	bool compressVector(const std::vector <uint8_t>* source, std::vector <uint8_t>* result, bool mkgzip);
	bool decompressVector(const std::vector <uint8_t>* source, std::vector <uint8_t>* result);

}


#endif