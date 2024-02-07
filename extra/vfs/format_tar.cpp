#include "./formats.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <filesystem>

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

enum struct EntryType : char {
	Normal = '0',
	LongLink = 'L',
	Directory = '5'
};

struct TarBasicHeader {
	std::string name;
	EntryType typeflag = EntryType::Normal;
	size_t size = 0;
	time_t modified = 0;
};

std::vector<uint8_t> serializeHeader(const TarBasicHeader& header) {

	const auto encodeTarInt = [](char* dest, int16_t destSize, size_t value) {

		std::string buffer;
		const size_t targetlen = destSize - 1;
		
		while (value > 0) {
			size_t remainder = value % 8;
			buffer = std::to_string(remainder) + buffer;
			value /= 8;
		}

		if (buffer.size() < targetlen) {
			const auto leftpad = std::string(targetlen - buffer.size(), '0');
			buffer.insert(buffer.begin(), leftpad.begin(), leftpad.end());
		}

		strncpy(dest, buffer.c_str(), destSize);
	};

	TarPosixHeader posixHeader;
	memset(&posixHeader, 0, sizeof(posixHeader));

	strcpy(posixHeader.magic, "ustar");
	strcpy(posixHeader.version, "00");
	posixHeader.typeflag = static_cast<std::underlying_type_t<EntryType>>(header.typeflag);

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

	const auto const headerDataPtr = reinterpret_cast<uint8_t*>(&posixHeader);
	return std::vector<uint8_t>(headerDataPtr, headerDataPtr + sizeof(posixHeader));
}

std::optional<TarBasicHeader> parseHeader(const TarPosixHeader& posixHeader) {

	//	check for empty block
	if (((uint8_t*)(&posixHeader))[0] == 0) {
		return std::nullopt;
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

	const auto parseTarInt = [](const char* field, size_t fieldsize) {
		return std::stoull(std::string(field, fieldsize), nullptr, 8);
	};

	auto originalChecksum = parseTarInt(posixHeader.chksum, sizeof(posixHeader.chksum));
	if (!(originalChecksum == checksumUnsigned || originalChecksum == checksumSigned)) {
		throw std::runtime_error("Tar checksum error");
	}

	TarBasicHeader header;

	const bool nameFieldFit = posixHeader.name[sizeof(posixHeader.name) - 1] == 0;
	const size_t nameFieldLength = nameFieldFit ? strlen(posixHeader.name) : sizeof(posixHeader.name);

	header.name = std::string(posixHeader.name, nameFieldLength);
	header.typeflag = static_cast<EntryType>(posixHeader.typeflag);

	//	decode file size
	header.size = parseTarInt(posixHeader.size, sizeof(posixHeader.size));

	//	get last modification date
	header.modified = parseTarInt(posixHeader.mtime, sizeof(posixHeader.mtime));

	return header;
}

size_t getPaddingSize(size_t contentSize) {
	if (contentSize == 0 || contentSize == tarBlockSize) return 0;
	return (tarBlockSize * ((contentSize / tarBlockSize) + 1)) - contentSize;
}

void paddBlock(std::vector<uint8_t>& buff) {
	const auto paddSize = getPaddingSize(buff.size());
	if (!paddSize) return;
	buff.resize(buff.size() + paddSize, 0);
}

void Tar::exportArchive(const std::string& path, FSQueue& queue) {

	auto outfile = std::fstream(path, std::ios::out | std::ios::binary);
	if (!outfile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for write", path, std::error_code(5L, std::generic_category()));
	}

	std::vector<uint8_t> writeBuff;

	while (queue.await()) {

		auto nextFile = queue.next();

		if (nextFile.name.size() >= sizeof(TarPosixHeader::name)) {

			TarBasicHeader longlinkHeader {
				"././@LongLink",
				EntryType::LongLink,
				nextFile.name.size(),
				nextFile.modified,
			};

			const auto longlinkSerialized = serializeHeader(longlinkHeader);
			writeBuff.insert(writeBuff.end(), longlinkSerialized.begin(), longlinkSerialized.end());
			writeBuff.insert(writeBuff.end(), nextFile.name.begin(), nextFile.name.end());
			paddBlock(writeBuff);
		}

		TarBasicHeader tarEntry {
			nextFile.name,
			EntryType::Normal,
			nextFile.buffer.size(),
			nextFile.modified,
		};

		const auto entryHeaderSerialized = serializeHeader(tarEntry);
		writeBuff.insert(writeBuff.end(), entryHeaderSerialized.begin(), entryHeaderSerialized.end());
		writeBuff.insert(writeBuff.end(), nextFile.buffer.begin(), nextFile.buffer.end());
		paddBlock(writeBuff);

		outfile.write((char*)writeBuff.data(), writeBuff.size());
		outfile.flush();
		writeBuff.erase(writeBuff.begin(), writeBuff.end());
	}

	writeBuff.resize(writeBuff.size() + (2 * tarBlockSize), 0);
	outfile.write((char*)writeBuff.data(), writeBuff.size());

	outfile.flush();
	outfile.close();
}
