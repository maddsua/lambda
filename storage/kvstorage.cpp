#include "./storage.hpp"

Lambda::Storage::KV::KV() {
	//	do nothing
}

Lambda::Storage::KV::~KV() {
	//	do nothing
}

bool Lambda::Storage::KV::has(const std::string key) {
	return this->data.find(key) != this->data.end();
}

Lambda::Storage::KVEntry Lambda::Storage::KV::get(std::string key) {
	if (!this->has(key)) return {};
	return this->data.at(key);
}

void Lambda::Storage::KV::set(std::string key, const std::string& value) {

	KVEntry newEntry;
	newEntry.value = value;
	newEntry.modified = time(nullptr);

	std::lock_guard <std::mutex> lock (threadLock);

	if (this->has(key)) {
		newEntry.created = get(key).created;
	} else newEntry.created = newEntry.modified;

	this->data[key] = std::move(newEntry);
}

bool Lambda::Storage::KV::del(const std::string key) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return false;
	this->data.erase(key);
	return true;
}

bool Lambda::Storage::KV::move(const std::string key, std::string newKey) {
	std::lock_guard <std::mutex> lock (threadLock);
	if (!this->has(key)) return false;
	this->data[newKey] = this->data.extract(key).mapped();
	return true;
}
