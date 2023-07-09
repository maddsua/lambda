#ifndef __LAMBDA_COMPRESSION__
#define __LAMBDA_COMPRESSION__

#include <stdint.h>
#include <vector>
#include "../lambda_private.hpp"

namespace Lambda::Compress {
	

	Lambda::Error brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality);
	Lambda::Error brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	Lambda::Error brotliDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	enum ZlibWinbits {
		ZLIB_HEADER_GZ = 26,
		ZLIB_HEADER_DEFLATE = 8,
		ZLIB_HEADER_RAW = -15,
		ZLIB_OPEN_AUTO = 32,
		ZLIB_OPEN_GZ = 16,
	};

	enum CompressLevels {
		ZLIB_LEVEL_STORE = 0,
		ZLIB_LEVEL_MEH = 1,
		ZLIB_LEVEL_QUICK = 2,
		ZLIB_LEVEL_REASONABLE = 6,
		ZLIB_LEVEL_GOOD = 7,
		ZLIB_LEVEL_SQUISHIN_GOOD = 8,
		ZLIB_LEVEL_HARD = 9,
	};

	Lambda::Error zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality, ZlibWinbits header);
	Lambda::Error zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	Lambda::Error zlibDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

};

#endif
