#include "./formats.hpp"

#include <cassert>
#include <cstring>

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

enum struct EntryMode : char {
	Normal = '0',
	LongLink = 'L',
	Directory = '5',
};

struct TarBasicHeader {
	std::string name;
	size_t size = 0;
	time_t modified = 0;
	EntryMode mode = EntryMode::Normal;
	bool isEof = false;
};

TarPosixHeader createHeader(const TarBasicHeader& header) {

	const auto encodeTarInt = [](char* buffer, int16_t buffsize, int64_t number) {

		//	actually don't need this memset here,
		//	the entire header structure will be all zeros
		//memset(buffer, 0, buffsize);

		char format[10];
		snprintf(format, sizeof(format), "%%0%io", buffsize - 1);

		char value[24];
		snprintf(value, sizeof(value), format, number);

		strncpy(buffer, value, buffsize - 1);
	};

	TarPosixHeader posixHeader;
	memset(&posixHeader, 0, sizeof(posixHeader));

	strcpy(posixHeader.magic, "ustar");
	strcpy(posixHeader.version, "00");
	posixHeader.typeflag = static_cast<std::underlying_type_t<TarBasicHeader>>(header.mode);

	strncpy(posixHeader.mode, "0100777", 7);
	strncpy(posixHeader.uid, "0000000", 7);
	strncpy(posixHeader.gid, "0000000", 7);

	strncpy(posixHeader.name, header.name.c_str(), sizeof(posixHeader.name) - 1);

	encodeTarInt(posixHeader.size, sizeof(posixHeader.mtime), header.size);
	encodeTarInt(posixHeader.mtime, sizeof(posixHeader.mtime), header.modified);

	//	calculate header checksum
	int64_t checksumSigned = 0;

	for (size_t i = 0; i < tarBlockSize; i++) {

		if (i >= 148 && i < 156) {
			checksumSigned += 32;
			continue;
		}

		checksumSigned += *((int8_t*)(&posixHeader) + i);
	}

	encodeTarInt(posixHeader.chksum, sizeof(posixHeader.chksum), checksumSigned);

	return posixHeader;
}

TarBasicHeader decodeHeader(const TarPosixHeader& posixHeader) {

	//	check for empty block
	if (((uint8_t*)(&posixHeader))[0] == 0) {
		TarBasicHeader temp;
		temp.isEof = true;
		return temp;
	}

	//	ensure ustar format
	if (strcmp(posixHeader.magic, "ustar")) {
		throw std::runtime_error("Tar record is not recognized. Only ustar is supported");
	}

	//	calculate header checksum
	uint64_t checksumUnsigned = 0;
	int64_t checksumSigned = 0;

	for (size_t i = 0; i < tarBlockSize; i++) {

		if (i >= 148 && i < 156) {
			checksumUnsigned += 32;
			checksumSigned += 32;
			continue;
		}

		checksumUnsigned += *((uint8_t*)(&posixHeader) + i);
		checksumSigned += *((int8_t*)(&posixHeader) + i);
	}

	auto parseTarInt = [](const std::string& field) {
		try { return std::stoull(field, nullptr, 8); }
			catch(...) { return 0ULL; }
	};

	auto originalChecksum = parseTarInt(std::string(posixHeader.chksum, sizeof(posixHeader.chksum)));
	if (!(originalChecksum == checksumUnsigned || originalChecksum == checksumSigned)) {
		throw std::runtime_error("Tar checksum error");
	}

	TarBasicHeader header;

	//	this is not required by the tar standard,
	//	but lambda is not a cli archive tool, and I don't want it
	//	to crash in case of a corrupted tar file
	auto nameSize = posixHeader.name[sizeof(posixHeader.name) - 1] == 0 ? strlen(posixHeader.name) : sizeof(posixHeader.name);

	header.name = std::string(posixHeader.name, nameSize);
	header.mode = static_cast<EntryMode>(posixHeader.typeflag);

	//	decode file size
	header.size = parseTarInt(std::string(posixHeader.size, sizeof(posixHeader.size)));

	//	get last modification date
	header.modified = parseTarInt(std::string(posixHeader.mtime, sizeof(posixHeader.mtime)));

	return header;
}

void Tar::exportArchive(const std::string& path, FSQueue& queue) {

}
