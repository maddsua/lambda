#include "./formats.hpp"
#include "../../core/polyfill/polyfill.hpp"
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

class InflatableReader {
	private:

		enum struct Compression {
			None, Gzip
		};

		std::fstream& m_readstream;
		std::vector <uint8_t> m_buff;
		std::optional<GzipStreamDecompressor> m_gz_decompressor;
		Compression m_compression = Compression::None;

		static const size_t bufferSize = 2 * 1024 * 1024;

		void m_bufferToContain(size_t expectedSize) {

			if (this->m_buff.size() >= expectedSize || this->m_readstream.eof()) {
				return;
			}

			auto& decompressor = this->m_gz_decompressor.value();

			std::vector<uint8_t> tempBuff(expectedSize);
			this->m_readstream.read(reinterpret_cast<char*>(tempBuff.data()), tempBuff.size());
			tempBuff.resize(this->m_readstream.gcount());

			auto nextDecompressed = decompressor.nextChunk(tempBuff);
			this->m_buff.insert(this->m_buff.end(), nextDecompressed.begin(), nextDecompressed.end());
		}

	public:
		InflatableReader(std::fstream& readStream) : m_readstream(readStream) {
			m_gz_decompressor = GzipStreamDecompressor();

			uint8_t magicBytes[8];
			this->m_readstream.read(reinterpret_cast<char*>(magicBytes), sizeof(magicBytes));
			this->m_readstream.clear();
			this->m_readstream.seekg(0, std::ios::beg);

			uint8_t refdataHeaderGzip[] = { 0x1f, 0x8b, 0x08 };
			if (!memcmp(magicBytes, refdataHeaderGzip, sizeof(refdataHeaderGzip))) {
				this->m_compression = Compression::Gzip;
			}
		}

		size_t readChunk(std::vector<uint8_t>& dest, size_t expectedSize) {

			switch (this->m_compression) {

				case Compression::Gzip: {

					this->m_bufferToContain(expectedSize);
					auto outSize = std::min(expectedSize, this->m_buff.size());

					dest.insert(dest.end(), this->m_buff.begin(), this->m_buff.begin() + outSize);
					this->m_buff.erase(this->m_buff.begin(), this->m_buff.begin() + outSize);

					return outSize;
				};

				default: {

					if (this->m_readstream.eof()) return 0;

					std::vector<uint8_t> tempBuff(expectedSize);
					this->m_readstream.read(reinterpret_cast<char*>(tempBuff.data()), tempBuff.size());

					const size_t bytesRead = this->m_readstream.gcount();
					dest.insert(dest.end(), tempBuff.begin(), tempBuff.begin() + bytesRead);

					return bytesRead;
				};
			}
		}

		void skipNext(size_t skipSize) {

			switch (this->m_compression) {

				case Compression::Gzip: {

					this->m_bufferToContain(skipSize);
					this->m_buff.erase(this->m_buff.begin(), this->m_buff.begin() + skipSize);

				} break;
				
				default: {

					if (this->m_readstream.eof()) return;
					this->m_readstream.seekg(skipSize, std::ios::cur);

				} break;
			}
		}

		bool isEof() const noexcept {
			const auto isCompressed = this->m_compression == Compression::None;
			const auto isFsEof = this->m_readstream.eof();
			return isCompressed ? isFsEof : isFsEof && !this->m_buff.size();
		}
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

TarBasicHeader parseHeader(const std::vector<uint8_t>& headerBuff) {

	//	check for empty block
	assert(headerBuff[0] != 0);

	//	check available space
	assert(headerBuff.size() >= sizeof(TarPosixHeader));

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

void Tar::exportArchive(const std::string& path, SyncQueue& queue) {

	auto outfile = std::fstream(path, std::ios::out | std::ios::binary);
	if (!outfile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for write", path, std::error_code(5L, std::generic_category()));
	}

	std::optional<GzipStreamCompressor> compressor;

	bool isGzipped = Strings::toLowerCase(static_cast<const std::string>(path)).ends_with("gz");
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

void Tar::importArchive(const std::string& path, SyncQueue& queue) {

	auto infile = std::fstream(path, std::ios::in | std::ios::binary);
	if (!infile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for read", path, std::error_code(5L, std::generic_category()));
	}

	auto reader = InflatableReader(infile);
	std::optional<std::string> nextLongLink;

	while (!reader.isEof()) {

		std::vector<uint8_t> rawHeader;
		auto bytesRead = reader.readChunk(rawHeader, sizeof(TarPosixHeader));

		if (!rawHeader[0] || bytesRead < rawHeader.size()) break;

		auto nextHeader = parseHeader(rawHeader);

		switch (nextHeader.typeflag) {

			case EntryType::LongLink: {

				if (nextLongLink.has_value()) {
					throw std::runtime_error("More than one longlink in tar sequence");
				}

				std::string linkName;

				std::vector<uint8_t> temp;
				reader.readChunk(temp, nextHeader.size);
				linkName.insert(linkName.end(), temp.begin(), temp.end());

				if (linkName.size() != nextHeader.size) {
					throw std::runtime_error("Incomplete linglink tar entry: \"" + nextHeader.name + "\"");
				}

				auto paddingSize = getPaddingSize(linkName.size());
				if (paddingSize) {
					reader.skipNext(paddingSize);
				}

				nextLongLink = linkName;

			} break;

			case EntryType::Normal: {

				std::vector<uint8_t> content;
				reader.readChunk(content, nextHeader.size);

				if (content.size() != nextHeader.size) {
					throw std::runtime_error("Incomplete file content for tar entry: \"" + nextHeader.name + "\"");
				}

				auto paddingSize = getPaddingSize(content.size());
				if (paddingSize) {
					reader.skipNext(paddingSize);
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
			
			default: break;
		}
	}

	queue.close();
}
