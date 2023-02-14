//	2023 maddsua
//	https://github.com/maddsua
//	c++ wrappers for zlib and brotli for simple buffer compression

//	Note:
//	zstd was removed from the project due to:
//	1. gzip being a more convinient format in case of data recovery
//	2. gzip is already used by the main server, so I see a little point
//	 in adding a third compression library here

#ifndef _maddsua_compression_wrapper
#define _maddsua_compression_wrapper

#include <string>

namespace lambda {

	namespace compression {

		std::string gzCompress(const std::string* data, bool gzipHeader);
		std::string gzDecompress(const std::string* data);

		std::string brCompress(const std::string* data);
		std::string brDecompress(const std::string* data);

	}
}


#endif