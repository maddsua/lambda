
#include "./webstorage.hpp"
#include "./driver.hpp"

using namespace Lambda::Storage;
using namespace Lambda::Storage::WebStorage;
using namespace Lambda::Storage::WebStorage::KV;

StorageInterface::StorageInterface() {

	this->driver = new Driver("StorageInterface.ldb");

	auto initData = this->driver->sync();
	if (initData.has_value()) {
		this->data = initData.value();
	}
}

StorageInterface::StorageInterface(const std::string& dbfile) {

	this->driver = new Driver(dbfile);

	auto initData = this->driver->sync();
	if (initData.has_value()) {
		this->data = initData.value();
	}
}

StorageInterface::~StorageInterface() {
	delete this->driver;
}

bool StorageInterface::hasItem(const std::string& key) const noexcept {
	return this->data.contains(key);
}

std::string StorageInterface::getItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	auto entry = this->data.find(key);
	if (entry == this->data.end()) return {};
	return entry->second;
}

void StorageInterface::setItem(const std::string& key, const std::string& value) {

	std::lock_guard <std::mutex> lock(this->mtlock);

	if (this->driver != nullptr) {
		this->driver->handleTransaction({ TransactionType::Put, &key, &value });
	}

	this->data[key] = value;
}

void StorageInterface::removeItem(const std::string& key) {

	std::lock_guard <std::mutex> lock(this->mtlock);

	if (!this->data.contains(key)) return;
	this->data.erase(key);

	if (this->driver != nullptr) {
		this->driver->handleTransaction({ TransactionType::Remove, &key });
	}
}

void StorageInterface::clear() {

	std::lock_guard <std::mutex> lock(this->mtlock);

	this->data.clear();

	if (this->driver != nullptr) {
		this->driver->handleTransaction({ TransactionType::Clear });
	}
}

size_t StorageInterface::size() const noexcept {
	return this->data.size();
}
