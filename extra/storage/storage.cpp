#include "./storage.hpp"

using namespace Lambda::Storage;

bool BaseStorage::hasItem(const std::string& key) const {
	return this->data.contains(key);
}

std::string BaseStorage::getItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	if (!this->data.contains(key)) return {};
	return this->data.find(key)->second;
}

void BaseStorage::setItem(const std::string& key, const std::string& value) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data[key] = value;
	handleTransaction(StorageTransaction::Tr_Set, &key, &value);
}

void BaseStorage::removeItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.erase(key);
	handleTransaction(StorageTransaction::Tr_Delete, &key, nullptr);
}

void BaseStorage::clear() {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.clear();
	handleTransaction(StorageTransaction::Tr_Clear, nullptr, nullptr);
}

size_t BaseStorage::length() const {
	return this->data.size();
}

size_t BaseStorage::size() const {
	return this->data.size();
}
