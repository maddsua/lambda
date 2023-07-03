#include "./kvstore.hpp"
#include <fstream>
#include <cstring>

using namespace Lambda;
using namespace Lambda::Storage;


const char magicString[] = "lambda kv store";

struct StoreFileHeader {
	char magicbytes[sizeof(magicString)];
	uint32_t headerSize;
};

struct StoreMainHeader {
	uint16_t version;
	uint8_t compression;
};

struct StoreRecordHeader {
	uint64_t created;
	uint64_t modified;
	uint32_t valueSize;
	uint16_t keySize;
	uint8_t type;
};

enum StoreRecordType {
	KVENTRY_DATA = 0,
	KVENTRY_META = 1,
};


bool KV::has(const std::string& key) {
	return this->data.find(key) != this->data.end();
}

KVMapEntry KV::get(const std::string& key) {
	std::lock_guard<std::mutex>lock(threadLock);
	if (!this->has(key)) return {};
	return this->data.at(key);
}

std::string KV::getValue(const std::string& key) {
	std::lock_guard<std::mutex>lock(threadLock);
	if (!this->has(key)) return {};
	return this->data.at(key).value;
}

void KV::set(const std::string& key, const std::string& value) {

	if (key.size() >= UINT16_MAX) throw Lambda::Error("Failed to set record: key size is too large (max: UINT16_MAX)");
	if (value.size() >= UINT32_MAX) throw Lambda::Error("Failed to set record: value size is too large (max: UINT32_MAX)");

	KVMapEntry newEntry;
	newEntry.value = value;
	newEntry.modified = time(nullptr);

	std::lock_guard<std::mutex>lock(threadLock);

	if (this->has(key)) {
		newEntry.created = get(key).created;
	} else newEntry.created = newEntry.modified;

	this->data[key] = std::move(newEntry);
}

bool KV::del(const std::string& key) {
	std::lock_guard<std::mutex>lock(threadLock);
	if (!this->has(key)) return false;
	this->data.erase(key);
	return true;
}

bool KV::move(const std::string& key, const std::string& newKey) {
	std::lock_guard<std::mutex>lock(threadLock);
	if (!this->has(key)) return false;
	this->data[newKey] = this->data.extract(key).mapped();
	return true;
}

std::vector<KVEntry> KV::entries() {

	std::vector<KVEntry> result;
	std::lock_guard<std::mutex>lock(threadLock);

	for (const auto& item : this->data) {

		KVEntry temp;

		temp.key = item.first;
		temp.value = item.second.value;
		temp.created = item.second.created;
		temp.modified = item.second.modified;
		
		result.push_back(std::move(temp));
	}

	return result;
}

void KV::exportStore(const char* filepath) {

	auto localfile = std::ofstream(filepath, std::ios::binary);
	if (!localfile.is_open()) throw Lambda::Error(std::string("Could not open file \"") + filepath + "\" for write");

	//	write file header
	StoreFileHeader fHeader;
	fHeader.headerSize = sizeof(StoreMainHeader);
	memcpy(fHeader.magicbytes, magicString, sizeof(magicString));
	localfile.write((const char*)&fHeader, sizeof(fHeader));

	StoreMainHeader mHeader;
	mHeader.version = 1;
	mHeader.compression = 0;
	localfile.write((const char*)&mHeader, sizeof(mHeader));

	//	lock data store and begin export
	std::lock_guard<std::mutex>lock(threadLock);

	for (const auto& item : this->data) {

		StoreRecordHeader rHeader;
		rHeader.created = item.second.created;
		rHeader.modified = item.second.modified;
		rHeader.keySize = item.first.size();
		rHeader.type = KVENTRY_DATA;
		rHeader.valueSize = item.second.value.size();

		localfile.write((const char*)&rHeader, sizeof(rHeader));
		localfile.write(item.first.data(), item.first.size());
		localfile.write(item.second.value.data(), item.second.value.size());
	}

	localfile.close();
}

void KV::importStore(const char* filepath) {

	auto localfile = std::ifstream(filepath, std::ios::binary);
	if (!localfile.is_open()) throw Lambda::Error(std::string("Could not open file \"") + filepath + "\" for read");

	StoreFileHeader fHeader;
	memset(&fHeader, 0, sizeof(fHeader));
	localfile.read((char*)&fHeader, sizeof(fHeader));

	if (strcmp(fHeader.magicbytes, magicString))
		throw Lambda::Error("The file does not appear to be a valid KV store file");

	StoreMainHeader mHeader;
	memset(&mHeader, 0, sizeof(mHeader));
	localfile.read((char*)&mHeader, fHeader.headerSize > sizeof(mHeader) ? sizeof(mHeader) : fHeader.headerSize);

	if (fHeader.headerSize > sizeof(mHeader)) throw Lambda::Error("Unsupported bundle version");

	while (!localfile.eof()) {
		StoreRecordHeader rheader;
		localfile.read((char*)&rheader, sizeof(rheader));

		if (rheader.keySize > UINT16_MAX || rheader.valueSize > UINT32_MAX)
			throw Lambda::Error("KV storage stream is damaged or otherwise invalid");

		std::string entryKey;
		entryKey.resize(rheader.keySize);

		KVMapEntry entry;
		entry.created = rheader.created;
		entry.modified = rheader.modified;
		entry.value.resize(rheader.valueSize);

		localfile.read((char*)entryKey.data(), rheader.keySize);
		localfile.read((char*)entry.value.data(), rheader.valueSize);
		this->data[entryKey] = entry;
	}
}
