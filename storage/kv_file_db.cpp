#include "./kv.hpp"
#include "../compress/streams.hpp"
#include <fstream>
#include <cstring>
#include <set>
#include <array>

using namespace Lambda;
using namespace Lambda::Compress;
using namespace Lambda::Storage;

const char magicString[] = "lambda kv snapshot";

struct SnapshotFileHeader {
	char magicbytes[sizeof(magicString)];
	uint32_t headerSize;
};

struct SnapshotMainHeader {
	uint16_t version;
	uint8_t compression;
};

struct SnapshotRecordHeader {
	uint64_t created;
	uint64_t modified;
	uint32_t valueSize;
	uint16_t keySize;
	uint8_t type;
};

enum SnapshotRecordType {
	KVENTRY_DATA = 0,
	KVENTRY_META = 1,
};

bool validateHeader(const SnapshotRecordHeader& header) {
	auto sizeErrorEmpty = !header.keySize || !header.valueSize;
	auto sizeErrorTooBig = header.keySize > UINT16_MAX || header.valueSize > UINT32_MAX;
	return !(sizeErrorEmpty || sizeErrorTooBig);
}

SnapshotRecordHeader mkSnapshotRecord(const std::pair<const std::string, KVMapEntry>& mapEntry) {
	SnapshotRecordHeader temp;
	temp.created = mapEntry.second.created;
	temp.modified = mapEntry.second.modified;
	temp.keySize = mapEntry.first.size();
	temp.type = KVENTRY_DATA;
	temp.valueSize = mapEntry.second.value.size();
	return temp;
}

Lambda::Error KV::exportSnapshot(const char* filepath, KVSnapshotCompress compression) {

	auto localfile = std::ofstream(filepath, std::ios::binary);
	if (!localfile.is_open()) return Lambda::Error(std::string("Could not open file \"") + filepath + "\" for write");

	//	write file header
	SnapshotFileHeader fHeader;
	fHeader.headerSize = sizeof(SnapshotMainHeader);
	memcpy(fHeader.magicbytes, magicString, sizeof(magicString));
	localfile.write((const char*)&fHeader, sizeof(fHeader));

	SnapshotMainHeader mHeader;
	mHeader.version = LAMBDAKV_VERSION;
	mHeader.compression = compression;
	localfile.write((const char*)&mHeader, sizeof(mHeader));

	//	lock the data state and begin export
	std::lock_guard<std::mutex>lock(threadLock);

	switch (compression) {

		case KVSNAP_COMPRESS_BR: {

			BrotliCompressStream br;

			//	setting a bit worse compression so it won't take 1452154631 years to complete if you have a big db
			BrotliEncoderSetParameter(br.stream, BROTLI_PARAM_QUALITY, 5);

			std::vector<uint8_t> tempBuff;
			std::array<uint8_t, br.chunk> writeBuff;

			auto next_in = tempBuff.data();
			size_t available_in = tempBuff.size();

			auto next_out = writeBuff.data();
			size_t available_out = writeBuff.size();

			auto mapIterator = this->data.begin();

			while (true) {

				if (available_in == 0) {

					tempBuff.clear();

					for (available_in == 0; mapIterator != this->data.end() && tempBuff.size() < br.chunk; mapIterator++) {
						
						const auto& item = *mapIterator;
						auto rHeader = mkSnapshotRecord(item);
						auto rHeaderStructPtr = (uint8_t*)&rHeader;

						tempBuff.insert(tempBuff.end(), rHeaderStructPtr, rHeaderStructPtr + sizeof(rHeader));
						tempBuff.insert(tempBuff.end(), item.first.begin(), item.first.end());
						tempBuff.insert(tempBuff.end(), item.second.value.begin(), item.second.value.end());
					}

					next_in = tempBuff.data();
					available_in = tempBuff.size();
				}

				if (!BrotliEncoderCompressStream(br.stream, mapIterator == this->data.end() ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS, &available_in, (const uint8_t**)&next_in, &available_out, &next_out, nullptr)) {
					return Lambda::Error("brotli encoder failed");
				}

				if (available_out == 0) {
					localfile.write((char*)writeBuff.data(), writeBuff.size());
					next_out = writeBuff.data();
					available_out = writeBuff.size();
				}

				if (BrotliEncoderIsFinished(br.stream)) {
					localfile.write((char*)writeBuff.data(), writeBuff.size() - available_out);
					available_out = 0;
					available_in = 0;
					break;
				}
			}

		} break;
		
		default: {

			for (const auto& item : this->data) {
				auto rHeader = mkSnapshotRecord(item);
				localfile.write((const char*)&rHeader, sizeof(rHeader));
				localfile.write(item.first.data(), item.first.size());
				localfile.write(item.second.value.data(), item.second.value.size());
			}

		} break;
	}

	localfile.close();

	return {};
}

bool decodeSnapshotBrRecord(std::vector<uint8_t>& buffer, KVMap& map) {

	auto slider = 0;
	auto sliderNext = sizeof(SnapshotRecordHeader);
	if (buffer.size() < sliderNext) return false;

	SnapshotRecordHeader rheader;
	memcpy(&rheader, buffer.data(), sliderNext);

	if (!validateHeader(rheader))
		throw Lambda::Error("KV snapshot stream is damaged");

	KVMapEntry entry;
	entry.created = rheader.created;
	entry.modified = rheader.modified;
	entry.value.resize(rheader.valueSize);

	slider = sliderNext;
	sliderNext += rheader.keySize;
	if (buffer.size() < sliderNext) return false;

	auto entryKey = std::string(buffer.begin() + slider, buffer.begin() + sliderNext);

	slider = sliderNext;
	sliderNext += rheader.valueSize;
	if (buffer.size() < sliderNext) return false;

	entry.value.insert(entry.value.end(), buffer.begin() + slider, buffer.begin() + sliderNext);
	buffer.erase(buffer.begin(), buffer.begin() + sliderNext);

	map[entryKey] = entry;

	return true;
}

Lambda::Error KV::importSnapshot(const char* filepath) {

	auto localfile = std::ifstream(filepath, std::ios::binary);
	if (!localfile.is_open()) return Lambda::Error(std::string("Could not open file \"") + filepath + "\" for read");

	SnapshotFileHeader fHeader;
	memset(&fHeader, 0, sizeof(fHeader));
	localfile.read((char*)&fHeader, sizeof(fHeader));

	if (strcmp(fHeader.magicbytes, magicString))
		return Lambda::Error("Cannot identify the file as a valid KV snapshot file");

	if (fHeader.headerSize > sizeof(SnapshotMainHeader))
		return Lambda::Error("Unsupported KV snapshot version");

	SnapshotMainHeader mHeader;
	memset(&mHeader, 0, sizeof(mHeader));
	localfile.read((char*)&mHeader, fHeader.headerSize > sizeof(mHeader) ? sizeof(mHeader) : fHeader.headerSize);

	if (mHeader.version > LAMBDAKV_VERSION)
		return Lambda::Error("Unsupported version, trying to open v" + std::to_string(mHeader.version) + ", but only v" + std::to_string(LAMBDAKV_VERSION) + " is supported");

	std::lock_guard<std::mutex>lock(threadLock);

	switch (mHeader.compression) {

		case KVSNAP_COMPRESS_BR: {

			std::vector<uint8_t> snapBuff;

			BrotliDecompressStream br;
			auto streamStatus = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;

			std::array<uint8_t, br.chunk> buffIn;
			std::array<uint8_t, br.chunk> buffOut;

			auto next_in = buffIn.data();
			size_t available_in = 0;

			auto next_out = buffOut.data();
			size_t available_out = buffOut.size();

			while (true) {

				auto currOutSize = buffOut.size() - available_out;

				switch (streamStatus) {

					case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {

						if (localfile.eof()) return Lambda::Error("Incomplete brotli stream");

						localfile.read((char*)buffIn.data(), buffIn.size());

						next_in = buffIn.data();
						available_in = localfile.gcount();

					} break;

					case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {

						snapBuff.insert(snapBuff.end(), buffOut.begin(), buffOut.begin() + currOutSize);
						next_out = buffOut.data();
						available_out = buffOut.size();

						try { while (decodeSnapshotBrRecord(snapBuff, this->data)); } catch (const std::exception& e) {
							return Lambda::Error(std::string("Could not parse db snapshot: ") + e.what());
						}

					} break;

					case BROTLI_DECODER_RESULT_SUCCESS: {

						if (currOutSize) snapBuff.insert(snapBuff.end(), buffOut.begin(), buffOut.begin() + currOutSize);
						
						try { while (decodeSnapshotBrRecord(snapBuff, this->data)); } catch (const std::exception& e) {
							return Lambda::Error(std::string("Could not parse db snapshot: ") + e.what());
						}
						
						available_in = 0;
						available_out = 0;

						return {};

					} break;

					default: {

						auto errcode = BrotliDecoderGetErrorCode(br.stream);
						return Lambda::Error(std::string("Brotli decompressor: ") + BrotliDecoderErrorString(errcode), errcode);

					} break;
				}

				streamStatus = BrotliDecoderDecompressStream(br.stream, &available_in, (const uint8_t**)&next_in, &available_out, &next_out, 0);
			}

		} break;

		default: {

			while (!localfile.eof()) {

				SnapshotRecordHeader rheader;
				localfile.read((char*)&rheader, sizeof(rheader));
				if (localfile.eof()) break;

				if (!validateHeader(rheader))
					return Lambda::Error("KV snapshot stream is damaged");

				std::string entryKey;
				entryKey.resize(rheader.keySize);

				KVMapEntry entry;
				entry.created = rheader.created;
				entry.modified = rheader.modified;
				entry.value.resize(rheader.valueSize);

				localfile.read((char*)entryKey.data(), rheader.keySize);
				if (localfile.eof()) break;

				localfile.read((char*)entry.value.data(), rheader.valueSize);
				
				this->data[entryKey] = entry;
			}

		} break;
	}

	return {};
}
