#include "./kv.hpp"
#include <set>

using namespace Lambda;
using namespace Lambda::Storage;


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

	if (!key.size()) throw Lambda::Error("Cannot set record: empty key");
	if (!value.size()) throw Lambda::Error("Cannot set record: empty value");

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
