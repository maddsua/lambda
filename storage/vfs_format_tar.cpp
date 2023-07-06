#include "./vfs.hpp"
#include "../compress/compress.hpp"
#include "../compress/streams.hpp"

#include <fstream>
#include <cstring>
#include <map>
#include <array>

using namespace Lambda;
using namespace Lambda::Compress;
using namespace Lambda::Storage;

/**
 * Exctracted the structure from https://ftp.gnu.org/gnu/tar/
 * I'm not including the entire tar project as a dependency,
 * I just don't really need it. Everything apart from this struct
 * is a custom code anyway
*/
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

//	it's kinda wrong to assign like this
//	 but the block size is equal to the header size anyway
static const uint16_t tarBlockSize = 512;
static const uint16_t tarHeaderSize = sizeof(TarPosixHeader);

struct TarBasicHeader {
	std::string name;
	size_t size = 0;
	time_t modified = 0;
	uint8_t type = 0;
	bool isEof = false;
};

/**
 * This is not a full list, just the part that is used in this project
 * I don't wanna make a "npm" package of this project,
 * so I'm not including the entire deps directly. Just the bare minimum
*/
enum TarHeaderValues {
	TAR_MODE_NORMAL = '0',
	TAR_MODE_LONGLINK = 'L',
	TAR_MODE_DIRECTORY = '5',
};

enum TarFileCompression {
	TAR_COMPRESSED_NONE = 0,
	TAR_COMPRESSED_GZIP = 1
};

TarBasicHeader decodeHeader(const TarPosixHeader& posixHeader) {

	//	check for empty block
	if (((uint8_t*)(&posixHeader))[0] == 0) {
		TarBasicHeader temp;
		temp.isEof = true;
		return temp;
	}

	//	ensure ustar format
	if (strcmp(posixHeader.magic, "ustar")) throw Lambda::Error("Tar record is not recognized. Only ustar is supported");

	//	calculate header checksum
	uint64_t checksumUnsigned = 0;
	int64_t checksumSigned = 0;

	for (size_t i = 0; i < tarHeaderSize; i++) {

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
	if (!(originalChecksum == checksumUnsigned || originalChecksum == checksumSigned))
		throw Lambda::Error("Tar checksum error");

	TarBasicHeader header;

	header.name = std::string(posixHeader.name, sizeof(posixHeader.name) - 1);
	header.type = posixHeader.typeflag;

	//	decode file size
	header.size = parseTarInt(std::string(posixHeader.size, sizeof(posixHeader.size)));

	//	get last modification date
	header.modified = parseTarInt(std::string(posixHeader.mtime, sizeof(posixHeader.mtime)));

	return header;
}

uint32_t getPaddingSize(size_t contentSize) {
	if (contentSize == 0 || contentSize == tarBlockSize) return 0ULL;
	return (tarBlockSize * ((contentSize / tarBlockSize) + 1)) - contentSize;
}

void VFS::importTar(std::ifstream& filestream) {

	//	determine if the file is gzipped
	auto tarCompression = TAR_COMPRESSED_NONE;

	uint8_t topheader[10];
	filestream.read((char*)topheader, sizeof(topheader));
	filestream.clear();
	filestream.seekg(0, std::ios::beg);

	uint8_t refdataHeaderGzip[] = { 0x1f, 0x8b, 0x08 };
	if (!memcmp(topheader, refdataHeaderGzip, sizeof(refdataHeaderGzip)))
		tarCompression = TAR_COMPRESSED_GZIP;

	//	start import itself
	std::vector<std::string> longlinks;

	auto dispatchFileRecord = [&longlinks, this](const TarBasicHeader& entry, const std::vector<uint8_t>& content) {

		switch (entry.type) {

			case TAR_MODE_NORMAL: {
				
				VirtualFile vfile;
				vfile.content = content;
				vfile.modified = entry.modified;
				vfile.created = entry.modified;

				std::lock_guard <std::mutex> lock(threadLock);

				if (longlinks.size()) {
					this->vfs[longlinks.front()] = vfile;
					longlinks.erase(longlinks.begin());
				} else this->vfs[entry.name] = vfile;

			} break;

				//	do nothing, we don't want dirs here
			case TAR_MODE_DIRECTORY:  break;

			case TAR_MODE_LONGLINK: {
				longlinks.push_back(std::string(content.begin(), content.end()));
			} break;
			
			default: throw Lambda::Error("Unknown tar record type for: \"" + entry.name + "\"");
		}
	};

	switch (tarCompression) {

		case TAR_COMPRESSED_GZIP: {

			auto zlib = ZlibDecompressStream(ZlibWinbits::ZLIB_OPEN_AUTO);
			int opres = Z_OK;

			std::array<uint8_t, zlib.chunk> buffIn;
			std::vector<uint8_t> buffTar;
			size_t cursor_out = 0;

			TarBasicHeader tempRecord;
			bool readingContent = false;
			size_t paddingSize = 0;

			do {
				
				//	supply zlib with new data
				filestream.read((char*)buffIn.data(), buffIn.size());
				zlib.stream->next_in = buffIn.data();
				zlib.stream->avail_in = filestream.gcount();

				//	undo the nasa-guy magic
				do {

					zlib.stream->avail_out = zlib.chunk;
					if ((buffTar.size() - cursor_out) < zlib.stream->avail_out)
						buffTar.resize(buffTar.size() + zlib.stream->avail_out);
					zlib.stream->next_out = buffTar.data() + cursor_out;

					opres = inflate(zlib.stream, Z_NO_FLUSH);
					if (opres < Z_OK || opres > Z_STREAM_END) throw Lambda::Error("inflate stream error", opres);

					cursor_out += zlib.chunk - zlib.stream->avail_out;

				} while (zlib.stream->avail_out == 0);

				//	undo the tar magic
				while (cursor_out > 0) {

					//	trim tar paddings
					if (paddingSize > 0 && cursor_out > 0) {

						auto trimSize = (cursor_out > paddingSize) ? paddingSize : (paddingSize - cursor_out);
						paddingSize -= trimSize;

						cursor_out -= trimSize;
						buffTar.erase(buffTar.begin(), buffTar.begin() + trimSize);
					}

					//	read tar record header
					if (!readingContent && cursor_out >= tarHeaderSize) {

						TarPosixHeader rawHeader;
						memcpy(&rawHeader, buffTar.data(), tarHeaderSize);

						tempRecord = decodeHeader(rawHeader);
						if (tempRecord.isEof) break;

						cursor_out -= tarHeaderSize;
						buffTar.erase(buffTar.begin(), buffTar.begin() + tarHeaderSize);

						if (tempRecord.size) readingContent = true;
					}

					//	read tar record payload (the file itself)
					else if (readingContent && cursor_out >= tempRecord.size) {

						auto content = std::vector<uint8_t>(buffTar.begin(), buffTar.begin() + tempRecord.size);
						dispatchFileRecord(tempRecord, content);
						paddingSize = getPaddingSize(content.size());

						cursor_out -= tempRecord.size;
						buffTar.erase(buffTar.begin(), buffTar.begin() + tempRecord.size);
						readingContent = false;
					}

					//	exit the loop if none of above
					else break;
				}

			} while (!filestream.eof() && opres != Z_STREAM_END);

			if (opres != Z_STREAM_END) throw Lambda::Error("inflate stream failed to properly finish", opres);

		} break;

		default: {

			while (!filestream.eof()) {

				TarPosixHeader rawHeader;
				filestream.read((char*)&rawHeader, sizeof(rawHeader));
				//	oops, we hit eof
				if (filestream.gcount() < sizeof(rawHeader)) break;

				//	decode header
				auto entry = decodeHeader(rawHeader);
				if (entry.isEof) break;

				std::vector<uint8_t> content;
				if (!filestream.eof() && entry.size) {

					content.resize(entry.size);
					filestream.read((char*)content.data(), content.size());

					if (content.size() != entry.size) throw Lambda::Error("Incomplete file content for tar entry: \"" + entry.name + "\"");

					//	account for block padding by moving file cursor forward
					auto paddingSize = getPaddingSize(content.size());

					if (paddingSize > 0)
						filestream.seekg(paddingSize, std::ios::cur);
				}

				dispatchFileRecord(entry, content);
			}

		} break;
	}
}

TarPosixHeader encodeHeader(const TarBasicHeader& header) {

	auto encodeTarInt = [](char* buffer, int16_t buffsize, int64_t number) {

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
	posixHeader.typeflag = header.type;

	strncpy(posixHeader.mode, "0100777", 7);
	strncpy(posixHeader.uid, "0000000", 7);
	strncpy(posixHeader.gid, "0000000", 7);

	strncpy(posixHeader.name, header.name.c_str(), sizeof(posixHeader.name) - 1);

	encodeTarInt(posixHeader.size, sizeof(posixHeader.mtime), header.size);
	encodeTarInt(posixHeader.mtime, sizeof(posixHeader.mtime), header.modified);

	//	calculate header checksum
	int64_t checksumSigned = 0;

	for (size_t i = 0; i < tarHeaderSize; i++) {

		if (i >= 148 && i < 156) {
			checksumSigned += 32;
			continue;
		}

		checksumSigned += *((int8_t*)(&posixHeader) + i);
	}
	encodeTarInt(posixHeader.chksum, sizeof(posixHeader.chksum), checksumSigned);

	return posixHeader;
}

void VFS::exportTar(std::ofstream& filestream, TarCompressionLayer compression) {

	std::lock_guard <std::mutex> lock(threadLock);

	switch (compression) {

		case TARCOMPRESS_GZIP: {

			auto zlib = ZlibCompressStream(CompressLevels::ZLIB_LEVEL_REASONABLE, ZlibWinbits::ZLIB_HEADER_GZ);

			bool eob = false;
			int opres = Z_OK;

			std::vector<uint8_t> tarBuffer;
			std::array<uint8_t, zlib.chunk> writeBuff;

			auto writeTarRecord = [&tarBuffer](const TarBasicHeader& header, const std::vector<uint8_t>& content) {
				
				auto rawHeader = encodeHeader(header);
				tarBuffer.insert(tarBuffer.end(), (uint8_t*)&rawHeader, (uint8_t*)&rawHeader + sizeof(rawHeader));

				tarBuffer.insert(tarBuffer.end(), content.begin(), content.end());

				auto paddingSize = getPaddingSize(header.size);
				if (!paddingSize) return;

				tarBuffer.resize(tarBuffer.size() + paddingSize, 0);
			};

			auto vfsIteraton = this->vfs.begin();

			do {

				tarBuffer.clear();

				for (; vfsIteraton != this->vfs.end() && tarBuffer.size() < zlib.chunk; vfsIteraton++) {

					const auto& entry = *vfsIteraton;

					TarBasicHeader tarFile;
					tarFile.name = entry.first;
					tarFile.modified = entry.second.modified;

					if (tarFile.name.size() >= sizeof(TarPosixHeader::name)) {

						auto tarLonglink = tarFile;

						tarLonglink.type = TAR_MODE_LONGLINK;
						tarLonglink.size = tarFile.name.size();
						tarLonglink.name = "././@LongLink";

						writeTarRecord(tarLonglink, std::vector<uint8_t>(tarFile.name.begin(), tarFile.name.end()));
					}

					tarFile.type = TAR_MODE_NORMAL;
					tarFile.size = entry.second.content.size();

					writeTarRecord(tarFile, entry.second.content);
				}

				eob = vfsIteraton == this->vfs.end();
				if (eob) tarBuffer.resize(tarBuffer.size() + (2 * tarBlockSize), 0);

				zlib.stream->avail_in = tarBuffer.size();
				zlib.stream->next_in = (uint8_t*)tarBuffer.data();

				do {

					zlib.stream->avail_out = writeBuff.size();
					zlib.stream->next_out = writeBuff.data();

					opres = deflate(zlib.stream, eob ? Z_FINISH : Z_NO_FLUSH);
					if (opres < 0) throw Lambda::Error("deflate stream error", opres);

					auto size_out = zlib.chunk - zlib.stream->avail_out;
					filestream.write((char*)writeBuff.data(), size_out);

				} while (zlib.stream->avail_out == 0);

			} while (!eob);

			if (opres != Z_STREAM_END) throw Lambda::Error("inflate stream failed to properly finish", opres);

		} break;
		
		default: {

			auto writeTarRecord = [&filestream](const TarBasicHeader& header, const std::vector<uint8_t>& content) {
				
				auto rawHeader = encodeHeader(header);
				filestream.write((char*)&rawHeader, sizeof(rawHeader));

				filestream.write((char*)content.data(), content.size());

				auto paddingSize = getPaddingSize(header.size);
				if (!paddingSize) return;
				
				std::vector<uint8_t> padding;
				padding.resize(paddingSize, 0);
				filestream.write((char*)padding.data(), padding.size());
			};

			for (const auto& entry : this->vfs) {

				TarBasicHeader tarFile;
				tarFile.name = entry.first;
				tarFile.modified = entry.second.modified;

				if (tarFile.name.size() >= sizeof(TarPosixHeader::name)) {

					auto tarLonglink = tarFile;

					tarLonglink.type = TAR_MODE_LONGLINK;
					tarLonglink.size = tarFile.name.size();
					tarLonglink.name = "././@LongLink";

					writeTarRecord(tarLonglink, std::vector<uint8_t>(tarFile.name.begin(), tarFile.name.end()));
				}

				tarFile.type = TAR_MODE_NORMAL;
				tarFile.size = entry.second.content.size();

				writeTarRecord(tarFile, entry.second.content);
			}

			std::vector<uint8_t> closingPadding;
			closingPadding.resize(2 * tarBlockSize, 0);
			filestream.write((char*)closingPadding.data(), closingPadding.size());

		} break;
	}
}
