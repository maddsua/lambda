#ifndef __LAMBDA_COMPRESSION__
#define __LAMBDA_COMPRESSION__

#include <stdint.h>
#include <vector>
#include "../lambda.hpp"

namespace Lambda::Compress {
	

	Lambda::Error brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality);
	Lambda::Error brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	Lambda::Error brotliDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	enum ZlibWinbits {
		ZLIB_HEADER_GZ = 26,
		ZLIB_HEADER_DEFLATE = 8,
		ZLIB_HEADER_RAW = -15,
		ZLIB_OPEN_AUTO = 32,
		ZLIB_OPEN_GZ= 16,
	};

	Lambda::Error zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality, ZlibWinbits header);
	Lambda::Error zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

	Lambda::Error zlibDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

};

#endif
