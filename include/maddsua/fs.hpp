#ifndef _maddsua_filesystem_wrapper
#define _maddsua_filesystem_wrapper

#include <string>

#define LAMBDA_FS_READ_CHUNK	(1048576)

namespace lambda {
	namespace fs {

		bool writeFileSync(const std::string path, const std::string* data);
		bool readFileSync(const std::string path, std::string* dest);
		
	}
}

#endif