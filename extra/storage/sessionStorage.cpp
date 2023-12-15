#include "./storage.hpp"

using namespace Lambda::Storage;

bool SessionStorage::hasItem(const std::string& key) const {
	return this->data.contains(key);
}

std::string SessionStorage::getItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	if (!this->data.contains(key)) return {};
	return this->data.find(key)->second;
}

void SessionStorage::setItem(const std::string& key, const std::string& value) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data[key] = value;
}

void SessionStorage::removeItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.erase(key);
}

void SessionStorage::clear() {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.clear();
}

size_t SessionStorage::length() const {
	return this->data.size();
}

size_t SessionStorage::size() const {
	return this->data.size();
}
