
#ifndef _maddsua_fs_wrapper
#define _maddsua_fs_wrapper


#include <stdint.h>
#include <stdio.h>
#include <dir.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <regex>

#define MADDSUA_FS_READ_CHUNK	(1048576)

namespace maddsuaFS {

	bool writeBinary(const std::string path, const std::vector<uint8_t>* data);
	bool readBinary(const std::string path, std::vector<uint8_t>* dest);
}


#endif