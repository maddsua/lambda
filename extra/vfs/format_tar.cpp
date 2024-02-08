#include "./formats.hpp"
#include "../../core/compression/compression.hpp"

#include <cassert>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <array>

using namespace Lambda;
using namespace Lambda::VFS;
using namespace Lambda::VFS::Formats;
using namespace Lambda::VFS::Formats::Tar;
using namespace Lambda::Compress;

const std::initializer_list<std::string> Tar::supportedExtensions {
	".tar", ".tar.gz", ".tgz"
};

const std::initializer_list<std::string> gzippedExtensions {
	".tar.gz", ".tgz"
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

void serializeHeader(const TarBasicHeader& header, std::vector<uint8_t>& destBuff) {

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

	strncpy(posixHeader.mode, "0100777", sizeof(posixHeader.mode));
	strncpy(posixHeader.uid, "0000000", sizeof(posixHeader.uid));
	strncpy(posixHeader.gid, "0000000", sizeof(posixHeader.gid));

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

	const auto headerDataPtr = reinterpret_cast<uint8_t*>(&posixHeader);
	destBuff.insert(destBuff.end(), headerDataPtr, headerDataPtr + sizeof(posixHeader));
}

TarBasicHeader parseHeader(const std::array<uint8_t, sizeof(TarPosixHeader)>& headerBuff) {

	//	check for empty block
	assert(headerBuff[0] != 0);

	const auto posixHeader = reinterpret_cast<const TarPosixHeader*>(headerBuff.data());

	//	ensure ustar format
	if (strcmp(posixHeader->magic, "ustar")) {
		throw std::runtime_error("Tar record is not recognized. Only ustar is supported");
	}

	//	calculate header checksum
	int64_t checksumCalculated = 0;

	for (size_t i = 0; i < headerBuff.size(); i++) {

		if (i >= 148 && i < 156) {
			checksumCalculated += 32;
			continue;
		}

		checksumCalculated += headerBuff[i];
	}

	const auto parseTarInt = [](const char* field, size_t fieldsize) {
		return std::stoll(std::string(field, fieldsize), nullptr, 8);
	};

	auto originalChecksum = parseTarInt(posixHeader->chksum, sizeof(posixHeader->chksum));
	if (originalChecksum != checksumCalculated) {
		throw std::runtime_error("Tar checksum error");
	}

	TarBasicHeader header;

	const bool nameFieldFit = posixHeader->name[sizeof(posixHeader->name) - 1] == 0;
	const size_t nameFieldLength = nameFieldFit ? strlen(posixHeader->name) : sizeof(posixHeader->name);

	header.name = std::string(posixHeader->name, nameFieldLength);
	header.typeflag = static_cast<EntryType>(posixHeader->typeflag);

	//	decode file size
	header.size = parseTarInt(posixHeader->size, sizeof(posixHeader->size));

	//	get last modification date
	header.modified = parseTarInt(posixHeader->mtime, sizeof(posixHeader->mtime));

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

	std::optional<GzipStreamCompressor> compressor;

	bool isGzipped = path.ends_with("gz");
	if (isGzipped) {
		compressor = GzipStreamCompressor(Quality::Noice);
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

			serializeHeader(longlinkHeader, writeBuff);
			writeBuff.insert(writeBuff.end(), nextFile.name.begin(), nextFile.name.end());
			paddBlock(writeBuff);
		}

		TarBasicHeader tarEntry {
			nextFile.name,
			EntryType::Normal,
			nextFile.buffer.size(),
			nextFile.modified,
		};

		serializeHeader(tarEntry, writeBuff);
		writeBuff.insert(writeBuff.end(), nextFile.buffer.begin(), nextFile.buffer.end());
		paddBlock(writeBuff);

		if (compressor.has_value()) {
			auto& compressorRef = compressor.value();
			auto compressedChunk = compressorRef.nextChunk(writeBuff);
			outfile.write((char*)compressedChunk.data(), compressedChunk.size());
		} else {
			outfile.write((char*)writeBuff.data(), writeBuff.size());
		}

		outfile.flush();
		writeBuff.erase(writeBuff.begin(), writeBuff.end());
	}

	writeBuff.resize(writeBuff.size() + (2 * tarBlockSize), 0);

	if (compressor.has_value()) {
		auto& compressorRef = compressor.value();
		auto compressedChunk = compressorRef.nextChunk(writeBuff, GzipStreamCompressor::StreamFlush::Finish);
		outfile.write((char*)compressedChunk.data(), compressedChunk.size());
	} else {
		outfile.write((char*)writeBuff.data(), writeBuff.size());
	}

	outfile.flush();
	outfile.close();
}

void Tar::importArchive(const std::string& path, FSQueue& queue) {

	auto infile = std::fstream(path, std::ios::in | std::ios::binary);
	if (!infile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for read", path, std::error_code(5L, std::generic_category()));
	}

	std::optional<GzipStreamDecompressor> decompressor;

	bool isGzipped = path.ends_with("gz");
	if (isGzipped) {
		decompressor = GzipStreamDecompressor();
	}

	std::optional<std::string> nextLongLink;

	std::vector<uint8_t> readBuff;

	while (!infile.eof() || readBuff.size()) {

		std::vector<uint8_t> tempBuffer(GzipStreamDecompressor::chunkSize);
		infile.read(reinterpret_cast<char*>(tempBuffer.data()), tempBuffer.size());

		if (decompressor.has_value()) {
			auto& decompressorRef = decompressor.value();
			auto decompressedChunk = decompressorRef.nextChunk(tempBuffer);
			readBuff.insert(readBuff.end(), decompressedChunk.begin(), decompressedChunk.end());
		} else {
			readBuff.insert(readBuff.end(), tempBuffer.begin(), tempBuffer.end());
		}

		if (readBuff.size() < sizeof(TarPosixHeader) && !infile.eof()) {
			continue;
		}

		std::array<uint8_t, sizeof(TarPosixHeader)> rawHeader;

		if (!rawHeader[0] || static_cast<size_t>(infile.gcount()) < rawHeader.size()) break;

		auto nextHeader = parseHeader(rawHeader);

		switch (nextHeader.typeflag) {

			case EntryType::LongLink: {

				if (nextLongLink.has_value()) {
					throw std::runtime_error("More than one longlink in tar sequence");
				}

				std::string linkName;
				linkName.resize(nextHeader.size);
				infile.read(linkName.data(), linkName.size());

				if (linkName.size() != nextHeader.size) {
					throw std::runtime_error("Incomplete file content for tar entry: \"" + nextHeader.name + "\"");
				}

				auto paddingSize = getPaddingSize(linkName.size());
				if (paddingSize) {
					infile.seekg(paddingSize, std::ios::cur);
				}

				nextLongLink = linkName;

			} break;

			case EntryType::Normal: {

				std::vector<uint8_t> content;
				content.resize(nextHeader.size);
				infile.read(reinterpret_cast<char*>(content.data()), content.size());

				if (content.size() != nextHeader.size) {
					throw std::runtime_error("Incomplete file content for tar entry: \"" + nextHeader.name + "\"");
				}

				auto paddingSize = getPaddingSize(content.size());
				if (paddingSize) {
					infile.seekg(paddingSize, std::ios::cur);
				}

				queue.push({
					content,
					nextHeader.modified,
					nextHeader.modified,
					nextLongLink.has_value() ? nextLongLink.value() : nextHeader.name
				});

				if (nextLongLink.has_value()) {
					nextLongLink = std::nullopt;
				}

			} break;
			
			default:
				break;
		}
	}

	queue.close();
}
