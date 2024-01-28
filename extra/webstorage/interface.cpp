
#include "./interface.hpp"
#include "./driver.hpp"

using namespace Lambda::Storage;
using namespace Lambda::Storage::WebStorage;

bool KVInterface::hasItem(const std::string& key) const noexcept {
	return this->data.contains(key);
}

std::string KVInterface::getItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	auto entry = this->data.find(key);
	if (entry == this->data.end()) return {};
	return entry->second;
}

void KVInterface::setItem(const std::string& key, const std::string& value) {

	std::lock_guard <std::mutex> lock(this->mtlock);

	if (this->driver != nullptr) {
		auto transaction = this->data.contains(key) ? TransactionType::Update : TransactionType::Create;
		this->driver->handleTransaction({ transaction, &key, &value });
	}

	this->data[key] = value;
}

void KVInterface::removeItem(const std::string& key) {

	std::lock_guard <std::mutex> lock(this->mtlock);

	if (this->driver != nullptr) {
		auto transaction = TransactionType::Remove;
		this->driver->handleTransaction({ transaction, &key });
	}

	this->data.erase(key);
}

void KVInterface::clear() {

	std::lock_guard <std::mutex> lock(this->mtlock);

	if (this->driver != nullptr) {
		this->driver->handleTransaction({ TransactionType::Clear });
	}

	this->data.clear();
}

size_t KVInterface::size() const noexcept {
	return this->data.size();
}

LocalStorage::LocalStorage() {

	this->driver = new KVDriver("localstorage.ldb");

	auto initData = this->driver->sync();
	if (initData.has_value()) {
		this->data = initData.value();
	}
}

LocalStorage::LocalStorage(const std::string& dbfile) {

	this->driver = new KVDriver(dbfile);

	auto initData = this->driver->sync();
	if (initData.has_value()) {
		this->data = initData.value();
	}
}

LocalStorage::~LocalStorage() {
	delete this->driver;
}
