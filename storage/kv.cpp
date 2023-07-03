#include "./kvstore.hpp"
#include <fstream>
#include <cstring>

using namespace Lambda;
using namespace Lambda::Storage;

const char magicString[] = "lambda kv bundle";

struct KVStoreFileHeader {
	char magicbytes[sizeof(magicString)];
	uint32_t headerSize;
};

bool KV::has(const std::string& key) {
	return this->data.find(key) != this->data.end();
}

KVEntry KV::get(const std::string& key) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return {};
	return this->data.at(key);
}

std::string KV::getValue(const std::string& key) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return {};
	return this->data.at(key).value;
}

void KV::set(const std::string& key, const std::string& value) {

	KVEntry newEntry;
	newEntry.value = value;
	newEntry.modified = time(nullptr);

	std::lock_guard <std::mutex> lock (threadLock);

	if (this->has(key)) {
		newEntry.created = get(key).created;
	} else newEntry.created = newEntry.modified;

	this->data[key] = std::move(newEntry);
}

bool KV::del(const std::string& key) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return false;
	this->data.erase(key);
	return true;
}

bool KV::move(const std::string& key, const std::string& newKey) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return false;
	this->data[newKey] = this->data.extract(key).mapped();
	return true;
}

Lambda::Error KV::exportBundle(const char* filepath, uint8_t compression) {

	auto localfile = std::ofstream(filepath, std::ios::binary);
	if (!localfile.is_open()) return { std::string("Could not open file \"") + filepath + "\" for write" };

	//	write file header
	KVStoreFileHeader bHeader;
	bHeader.headerSize = sizeof(KVStoreHeader);
	memcpy(bHeader.magicbytes, magicString, sizeof(magicString));
	localfile.write((const char*)&bHeader, sizeof(bHeader));

	KVStoreHeader sHeader;
	sHeader.version = 1;
	sHeader.compression = compression;
	localfile.write((const char*)&sHeader, sizeof(sHeader));

	//	lock data store and begin export
	std::lock_guard <std::mutex> lock (threadLock);

	for (const auto& item : this->data) {

		KVRecordHeader header;
		header.created = item.second.created;
		header.modified = item.second.modified;
		header.keySize = item.first.size();
		header.type = KVENTRY_DATA;
		header.valueSize = item.second.value.size();

		localfile.write((const char*)&header, sizeof(header));
		localfile.write(item.first.data(), item.first.size());
		localfile.write(item.second.value.data(), item.second.value.size());
	}

	localfile.close();
	return {};
}

Lambda::Error KV::importStore(const char* filepath) {

	auto localfile = std::ifstream(filepath, std::ios::binary);
	if (!localfile.is_open()) return { std::string("Could not open file \"") + filepath + "\" for read" };

	KVStoreFileHeader bHeader;
	memset(&bHeader, 0, sizeof(bHeader));
	localfile.read((char*)&bHeader, sizeof(bHeader));

	KVStoreHeader sHeader;
	memset(&sHeader, 0, sizeof(sHeader));
	localfile.read((char*)&sHeader, bHeader.headerSize > sizeof(sHeader) ? sizeof(sHeader) : bHeader.headerSize);

	if (bHeader.headerSize > sizeof(sHeader)) return { "Unsupported bundle version" };

	while (!localfile.eof()) {
		KVRecordHeader header;
		localfile.read((char*)&header, sizeof(header));

		std::string entryKey;
		entryKey.resize(header.keySize);

		KVEntry entry;
		entry.created = header.created;
		entry.modified = header.modified;
		entry.value.resize(header.valueSize);

		localfile.read((char*)entryKey.data(), header.keySize);
		localfile.read((char*)entry.value.data(), header.valueSize);
		this->data[entryKey] = entry;
	}

	return {};
}
