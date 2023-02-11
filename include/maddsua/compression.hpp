//	2023 maddsua
//	https://github.com/maddsua
//	c++ wrappers for zlib, brotli and zstd for simple buffer compression

#ifndef _maddsua_compression_wrapper
#define _maddsua_compression_wrapper

#include <string>

namespace lambda {
	namespace compression {
		std::string gzCompress(const std::string* data, bool gzipHeader);
		std::string gzDecompress(const std::string* data);

		std::string brCompress(const std::string* data);
		std::string brDecompress(const std::string* data);

		std::string zstdCompress(const std::string* data);
		std::string zstdDecompress(const std::string* data);
	}
}


#endif