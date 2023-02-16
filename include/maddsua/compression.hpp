/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: C++ wrappers for zlib and brotli for simple buffer compression
*/

//	Note:
//	zstd was removed from the project due to:
//	1. gzip being a more convinient format in case of data recovery
//	2. gzip is already used by the main server, so I see a little point
//	 in adding a third compression library here

#ifndef H_MADDSUA_LAMBDA_COMPRESSION
#define H_MADDSUA_LAMBDA_COMPRESSION

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