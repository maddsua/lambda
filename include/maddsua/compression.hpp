#ifndef _maddsua_compression_wrapper
#define _maddsua_compression_wrapper

#include <string>

namespace lambda {
	namespace compression {
		bool gzCompress(const std::string* plain, std::string* compressed, bool gzipHeader);
		bool gzDecompress(const std::string* compressed, std::string* plain);

		bool brCompress(const std::string* plain, std::string* encoded);
		bool brDecompress(const std::string* encoded, std::string* plain);
	}
}


#endif