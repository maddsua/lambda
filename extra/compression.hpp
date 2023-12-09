#ifndef _OCTOPUSS_EXTRA_COMPRESSION_
#define _OCTOPUSS_EXTRA_COMPRESSION_

#include <stdint.h>
#include <vector>

namespace Compress {

	enum class Quality {
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

	enum class ZlibSetHeader {
		Zheader_GZ = 26,
		Zheader_Defalte = 8,
		Zheader_RAW = -15,
	};

	std::vector<uint8_t> zlibCompressBuffer(const std::vector<uint8_t>& input, Quality quality, ZlibSetHeader header);
	std::vector<uint8_t> zlibDecompressBuffer(const std::vector<uint8_t>& input);

};

#endif
