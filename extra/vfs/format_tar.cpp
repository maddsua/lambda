#include "./formats.hpp"

#include <cassert>

using namespace Lambda;
using namespace Lambda::VFS;
using namespace Lambda::VFS::Formats;
using namespace Lambda::VFS::Formats::Tar;

const std::initializer_list<std::string> Tar::supportedExtensions {
	".tar", ".tar.gz", ".tgz"
};

struct TarPosixHeader {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char zero[12];
};

static const uint16_t tarBlockSize = 512;
static_assert(sizeof(TarPosixHeader) == tarBlockSize);



void Tar::exportArchive(const std::string& path, FSQueue& queue) {

}
