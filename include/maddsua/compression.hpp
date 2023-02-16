/*
	zlib and brotli C++ wrappers for simple buffer compression
	2023 maddsua
	https://github.com/maddsua/
*/


#ifndef H_MADDSUA_COMPRESSION
#define H_MADDSUA_COMPRESSION

	#include <string>

	namespace maddsua {

		std::string gzCompress(const std::string* data, bool gzipHeader);
		std::string gzDecompress(const std::string* data);

		std::string brCompress(const std::string* data);
		//std::string brDecompress(const std::string* data);
	}

#endif