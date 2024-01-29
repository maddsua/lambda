#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION__
#define __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION__

#include <stdint.h>
#include <vector>

namespace Lambda::Compress {

	enum struct Quality {
		Store = 0,
		Barely = 1,
		Fast = 2,
		Reasonable = 3,
		Good = 4,
		Better = 5,
		Noice = 6,
		Real_noice = 7,
		Squishin = 8,
		Max = 9,
	};

	std::vector<uint8_t> brotliCompressBuffer(const std::vector<uint8_t>& input, Quality quality);
	std::vector<uint8_t> brotliDecompressBuffer(const std::vector<uint8_t>& input);

	enum struct ZlibSetHeader {
		Gzip = 26,
		Defalte = 8,
		Raw = -15,
	};

	std::vector<uint8_t> zlibCompressBuffer(const std::vector<uint8_t>& input, Quality quality, ZlibSetHeader header);
	std::vector<uint8_t> zlibDecompressBuffer(const std::vector<uint8_t>& input);

};

#endif
