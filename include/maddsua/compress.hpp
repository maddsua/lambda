/*

	This wrapper was created so compression just works.

	Requirements:
		zlib: https://github.com/madler/zlib
	
*/

#include <stdint.h>
#include <string>

#ifndef _maddsua_compression_wrapper
#define _maddsua_compression_wrapper

#define MCOMP_Z_HEADER_Z		(8)
#define MCOMP_Z_HEADER_GZ		(26)
#define MCOMP_Z_DECOMP_GZ		(16)
#define MCOMP_Z_DECOM_AUTO		(32)
#define MCOMP_Z_RAW				(-15)
#define MCOMP_Z_MEMORY			(9)
#define MCOMP_Z_EXPECT_RATIO	(3)
#define MCOMP_Z_CHUNK			(131072)	//	128k

#define MCOMP_BR_CHUNK			(131072)	//	128k
#define MCOMP_BR_EXPECT_RATIO	(3)


namespace maddsua {

	bool gzCompress(const std::string* plain, std::string* compressed, bool gzipHeader);
	bool gzDecompress(const std::string* compressed, std::string* plain);

	bool brCompress(const std::string* plain, std::string* encoded);
	bool brDecompress(const std::string* encoded, std::string* plain);

}


#endif