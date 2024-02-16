#include "./formats.hpp"
#include "../../core/polyfill/polyfill.hpp"
#include "../../buildopts.hpp"

#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION
	#include "../../core/compression/compression.hpp"
#endif

#include <cassert>
#include <array>
#include <cstring>
#include <fstream>
#include <filesystem>

using namespace Lambda;
using namespace Lambda::VFS;
using namespace Lambda::VFS::Formats;
using namespace Lambda::VFS::Formats::Tar;

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

size_t getPaddingSize(size_t contentSize) {
	if (contentSize == 0 || contentSize == tarBlockSize) return 0;
	return (tarBlockSize * ((contentSize / tarBlockSize) + 1)) - contentSize;
}

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

#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION

enum struct TarCompression {
	None, Gzip
};

class InflatableReader {
	private:
		std::ifstream& m_readstream;
		std::vector <uint8_t> m_buff;
		TarCompression m_compression;

		Compress::GzipStreamDecompressor* m_gz_strm = nullptr;

		static const size_t bufferSize = 2 * 1024 * 1024;

		/**
		 * Read end decompress enough data to contain expectedSize
		*/
		void m_decompressToContain(size_t expectedSize) {

			if (!this->m_gz_strm) {
				throw std::runtime_error("InflatableReader::m_gz_strm is null");
			}

			if (this->m_buff.size() >= expectedSize || this->m_readstream.eof() || this->m_gz_strm->isDone()) {
				return;
			}

			std::vector<uint8_t> tempBuff(expectedSize);
			this->m_readstream.read(reinterpret_cast<char*>(tempBuff.data()), tempBuff.size());
			tempBuff.resize(this->m_readstream.gcount());

			auto nextDecompressed = this->m_gz_strm->nextChunk(tempBuff);
			this->m_buff.insert(this->m_buff.end(), nextDecompressed.begin(), nextDecompressed.end());
		}

	public:
		InflatableReader(std::ifstream& readStream, TarCompression usedCompression)
		: m_readstream(readStream), m_compression(usedCompression) {
			if (usedCompression == TarCompression::Gzip) {
				this->m_gz_strm = new Compress::GzipStreamDecompressor();
			}
		}
		~InflatableReader() {
			if (this->m_gz_strm) {
				delete this->m_gz_strm;
			}
		}

		std::vector<uint8_t> readChunk(size_t expectedSize) {

			switch (this->m_compression) {

				case TarCompression::Gzip: {

					this->m_decompressToContain(expectedSize);
					const auto outSize = std::min(expectedSize, this->m_buff.size());

					const auto result = std::vector<uint8_t>(this->m_buff.begin(), this->m_buff.begin() + outSize);
					this->m_buff.erase(this->m_buff.begin(), this->m_buff.begin() + outSize);

					return result;
				};

				default: {

					if (this->m_readstream.eof()) {
						return {};
					}

					std::vector<uint8_t> tempBuff(expectedSize);
					this->m_readstream.read(reinterpret_cast<char*>(tempBuff.data()), tempBuff.size());

					const auto bytesRead = this->m_readstream.gcount();
					return std::vector<uint8_t>(tempBuff.begin(), tempBuff.begin() + bytesRead);
				};
			}
		}

		std::string readTextChunk(size_t expectedSize) {
			const auto temp = this->readChunk(expectedSize);
			return std::string(temp.begin(), temp.end());
		}

		void skipNext(size_t skipSize) {

			switch (this->m_compression) {

				case TarCompression::Gzip: {

					this->m_decompressToContain(skipSize);
					this->m_buff.erase(this->m_buff.begin(), this->m_buff.begin() + skipSize);

				} break;
				
				default: {

					if (this->m_readstream.eof()) return;
					this->m_readstream.seekg(skipSize, std::ios::cur);

				} break;
			}
		}

		bool isEof() const noexcept {
			const auto isCompressed = this->m_compression == TarCompression::None;
			const auto isFsEof = this->m_readstream.eof();
			return isCompressed ? isFsEof : isFsEof && !this->m_buff.size();
		}
};

class DeflatableWriter {
	private:
		std::ofstream& m_readstream;
		TarCompression m_compression;

		Compress::GzipStreamCompressor* m_gz_strm = nullptr;

	public:
		DeflatableWriter(std::ofstream& readStream, TarCompression usedCompression)
		: m_readstream(readStream), m_compression(usedCompression) {

			if (usedCompression == TarCompression::Gzip) {
				this->m_gz_strm = new Compress::GzipStreamCompressor(Compress::Quality::Noice);
			}
		}
		~DeflatableWriter() {
			if (this->m_gz_strm) {
				delete this->m_gz_strm;
			}
		}

		void writeChunk(std::vector<uint8_t> data) {

			const auto paddingSize = getPaddingSize(data.size());
			if (paddingSize) {
				data.resize(data.size() + paddingSize, 0);
			}

			switch (this->m_compression) {

				case TarCompression::Gzip: {

					if (!this->m_gz_strm) {
						throw std::runtime_error("InflatableReader::m_gz_decompressor is null");
					}

					auto compressedChunk = this->m_gz_strm->nextChunk(data);
					this->m_readstream.write(reinterpret_cast<char*>(compressedChunk.data()), compressedChunk.size());

				} break;
				
				default: {

					this->m_readstream.write(reinterpret_cast<char*>(data.data()), data.size());

				} break;
			}

			this->m_readstream.flush();
		}

		void endStream() {

			auto trailingEmptyBlock = std::vector<uint8_t>(2 * tarBlockSize, 0);

			switch (this->m_compression) {

				case TarCompression::Gzip: {

					if (!this->m_gz_strm) {
						throw std::runtime_error("DeflatableWriter::m_gz_strm is null");
					}

					auto compressedChunk = this->m_gz_strm->nextChunk(trailingEmptyBlock, Compress::GzipStreamCompressor::StreamFlush::Finish);
					this->m_readstream.write(reinterpret_cast<char*>(compressedChunk.data()), compressedChunk.size());

				} break;

				default: {

					this->m_readstream.write(reinterpret_cast<char*>(trailingEmptyBlock.data()), trailingEmptyBlock.size());

				} break;
			}

			this->m_readstream.flush();
		}
};

#else

class PlainFSReader {
	private:
		std::ifstream& m_readstream;

	public:
		PlainFSReader(std::ifstream& readStream) : m_readstream(readStream) {}

		std::vector<uint8_t> readChunk(size_t expectedSize) {

			if (this->m_readstream.eof()) {
				return {};
			}

			std::vector<uint8_t> tempBuff(expectedSize);
			this->m_readstream.read(reinterpret_cast<char*>(tempBuff.data()), tempBuff.size());

			const auto bytesRead = this->m_readstream.gcount();
			return std::vector<uint8_t>(tempBuff.begin(), tempBuff.begin() + bytesRead);
		}

		std::string readTextChunk(size_t expectedSize) {
			const auto temp = this->readChunk(expectedSize);
			return std::string(temp.begin(), temp.end());
		}

		void skipNext(size_t skipSize) {
			if (this->m_readstream.eof()) return;
			this->m_readstream.seekg(skipSize, std::ios::cur);
		}

		bool isEof() const noexcept {
			return this->m_readstream.eof();
		}
};

class PlainFSWriter {
	private:
		std::ofstream& m_readstream;

	public:
		PlainFSWriter(std::ofstream& readStream) : m_readstream(readStream) {}

		void writeChunk(std::vector<uint8_t> data) {

			const auto paddingSize = getPaddingSize(data.size());
			if (paddingSize) {
				data.resize(data.size() + paddingSize, 0);
			}

			this->m_readstream.write(reinterpret_cast<char*>(data.data()), data.size());
			this->m_readstream.flush();
		}

		void endStream() {
			auto trailingEmptyBlock = std::vector<uint8_t>(2 * tarBlockSize, 0);
			this->m_readstream.write(reinterpret_cast<char*>(trailingEmptyBlock.data()), trailingEmptyBlock.size());
			this->m_readstream.flush();
		}
};

#endif

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
	return std::vector<uint8_t>(headerDataPtr, headerDataPtr + sizeof(posixHeader));
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

void Tar::exportArchive(const std::string& path, SyncQueue& queue) {

	auto outfile = std::ofstream(path, std::ios::binary);
	if (!outfile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for write", path, std::error_code(5L, std::generic_category()));
	}


	#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION
		bool isGzipped = Strings::toLowerCase(static_cast<const std::string>(path)).ends_with("gz");
		auto writer = DeflatableWriter(outfile, isGzipped ? TarCompression::Gzip : TarCompression::None);
	#else
		auto writer = PlainFSWriter(outfile);
	#endif

	while (queue.await()) {

		auto nextFile = queue.next();

		if (nextFile.name.size() >= sizeof(TarPosixHeader::name)) {

			TarBasicHeader longlinkHeader {
				"././@LongLink",
				EntryType::LongLink,
				nextFile.name.size(),
				nextFile.modified,
			};

			writer.writeChunk(serializeHeader(longlinkHeader));
			writer.writeChunk(std::vector<uint8_t>(nextFile.name.begin(), nextFile.name.end()));
		}

		TarBasicHeader tarEntry {
			nextFile.name,
			EntryType::Normal,
			nextFile.buffer.size(),
			nextFile.modified,
		};

		writer.writeChunk(serializeHeader(tarEntry));
		writer.writeChunk(nextFile.buffer);
	}

	writer.endStream();
}

void Tar::importArchive(const std::string& path, SyncQueue& queue) {

	auto infile = std::ifstream(path, std::ios::binary);
	if (!infile.is_open()) {
		throw std::filesystem::filesystem_error("Could not open file for read", path, std::error_code(5L, std::generic_category()));
	}


	#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION
		bool isGzipped = Strings::toLowerCase(static_cast<const std::string>(path)).ends_with("gz");
		auto reader = InflatableReader(infile, isGzipped ? TarCompression::Gzip : TarCompression::None);
	#else
		auto reader = PlainFSReader(infile);
	#endif

	std::optional<std::string> nextLongLink;

	while (!reader.isEof()) {

		const auto rawHeader = reader.readChunk(sizeof(TarPosixHeader));
		if (rawHeader.size() < sizeof(TarPosixHeader) || !rawHeader[0]) break;

		const auto nextHeader = parseHeader(rawHeader);

		switch (nextHeader.typeflag) {

			case EntryType::LongLink: {

				if (nextLongLink.has_value()) {
					throw std::runtime_error("More than one longlink in tar sequence");
				}

				const auto linkName = reader.readTextChunk(nextHeader.size);
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

				const auto content = reader.readChunk(nextHeader.size);
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
