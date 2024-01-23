#include "./storage.hpp"
#include "../../core/encoding/encoding.hpp"

#include <stdexcept>
#include <iterator>
#include <filesystem>

using namespace Lambda::Storage;

enum struct DiskLogWriteOps {
	StartBlock = 0x06,
	FieldSep = 0x1f,
	BlockEnd = 0x00,
	Set = 0x01,
	Del = 0x02,
	Drop = 0x03,
};

LocalStorage::LocalStorage() {

	this->dbFileName = new std::string("localstorage.ldb");
	this->stats = new StorageTrStats();

	loadFile(*this->dbFileName);
}

LocalStorage::LocalStorage(const std::string& dbfile) {

	this->dbFileName = new std::string(dbfile);
	this->stats = new StorageTrStats();

	loadFile(*this->dbFileName);
}

LocalStorage::~LocalStorage() {
	this->writeStream.close();
	delete this->dbFileName;
	delete this->stats;
}

void LocalStorage::loadFile(const std::string& dbfile) {

	if (std::filesystem::exists(dbfile)) {
		
		auto readfile = std::ifstream(dbfile, std::ios::binary);
		if (!readfile.is_open()) throw std::runtime_error("Could not open \"" + dbfile + "\" for read");

		readfile.seekg(0, std::ios::end);
		auto fileSize = readfile.tellg();
		readfile.seekg(0, std::ios::beg);

		std::vector<uint8_t> rawcontent;
		rawcontent.reserve(fileSize);

		rawcontent.insert(rawcontent.begin(), std::istream_iterator<uint8_t>(readfile), std::istream_iterator<uint8_t>());
		readfile.close();

		std::vector<std::vector<uint8_t>> messages;

		if (rawcontent.size()) {
			size_t lastMessageStart = 0;
			for (size_t i = lastMessageStart; i < rawcontent.size(); i++) {
				if (rawcontent[i] == DiskLogWriteOps::BlockEnd) {
					messages.push_back(std::vector<uint8_t>(rawcontent.begin() + lastMessageStart, rawcontent.begin() + i));
					lastMessageStart = i + 1;
				}
			}
		}

		for (size_t i = 0; i < messages.size(); i++) {

			const auto& entry = messages[i];

			if (entry.at(0) != DiskLogWriteOps::StartBlock)
				throw std::runtime_error("Invalid start byte in message " + std::to_string(i));

			auto getFields = [&entry]() {
				std::vector<std::vector<uint8_t>> messageFields;
				size_t lastFieldStart = 2;
				for (size_t i = lastFieldStart; i < entry.size(); i++) {
					if (entry[i] == DiskLogWriteOps::FieldSep) {
						messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.begin() + i));
						lastFieldStart = i + 1;
					}
				}
				messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.end()));
				return messageFields;
			};

			switch (entry.at(1)) {

				case DiskLogWriteOps::Set: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					auto& value = fields.at(1);
					auto decodedValue = Encoding::fromBase64(std::string(value.begin(), value.end()));

					this->data[std::string(decodedKey.begin(), decodedKey.end())] = std::string(decodedValue.begin(), decodedValue.end());

				} break;

				case DiskLogWriteOps::Del: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					this->data.erase(std::string(decodedKey.begin(), decodedKey.end()));

				} break;

				case DiskLogWriteOps::Drop: {
					this->data.clear();
				} break;

				default: throw std::runtime_error("Invalid action byte in message " + std::to_string(i));
			}
		}
	}

	this->rebuildStorageSnapshot();
}

void LocalStorage::handleTransaction(StorageTransaction tra, const std::string* key, const std::string* value) {

	std::vector<uint8_t> writeBuff;

	switch (tra) {

		case StorageTransaction::Set: {

			if (!key) throw std::runtime_error("key pointer should be defined for write operation");
			if (!value) throw std::runtime_error("value pointer should be defined for write operation");

			writeBuff.insert(writeBuff.end(), {
				DiskLogWriteOps::StartBlock,
				DiskLogWriteOps::Set,
			});

			auto encodedKey = Encoding::toBase64(std::vector<uint8_t>(key->begin(), key->end()));
			writeBuff.insert(writeBuff.end(), encodedKey.begin(), encodedKey.end());

			writeBuff.insert(writeBuff.end(), { DiskLogWriteOps::FieldSep });
			
			auto encodedValue = Encoding::toBase64(std::vector<uint8_t>(value->begin(), value->end()));
			writeBuff.insert(writeBuff.end(), encodedValue.begin(), encodedValue.end());
			
			writeBuff.insert(writeBuff.end(), { DiskLogWriteOps::BlockEnd });

		} break;

		case StorageTransaction::Remove: {

			if (!key) throw std::runtime_error("key pointer should be defined for delete operation");

			writeBuff.insert(writeBuff.end(), {
				DiskLogWriteOps::StartBlock,
				DiskLogWriteOps::Del,
			});

			auto encodedKey = Encoding::toBase64(std::vector<uint8_t>(key->begin(), key->end()));
			writeBuff.insert(writeBuff.end(), encodedKey.begin(), encodedKey.end());

			writeBuff.insert(writeBuff.end(), { DiskLogWriteOps::BlockEnd });

			if (this->stats != nullptr) this->stats->deletions++;

		} break;

		case StorageTransaction::Clear: {

			writeBuff.insert(writeBuff.end(), {
				DiskLogWriteOps::StartBlock,
				DiskLogWriteOps::Drop,
				DiskLogWriteOps::BlockEnd
			});
			
		} break;

		default: throw std::runtime_error("unknown transaction type");
	}

	this->writeStream.write((char*)writeBuff.data(), writeBuff.size());
	this->writeStream.flush();

	if (this->stats != nullptr && this->stats->deletions > 1000) {
		this->stats->deletions = 0;
		this->rebuildStorageSnapshot();
	}
}

void LocalStorage::rebuildStorageSnapshot() {

	if (this->dbFileName == nullptr) throw std::runtime_error("LocalStorage::dbFileName is undefined");

	if (this->writeStream.is_open()) {
		this->writeStream.close();
	}

	this->writeStream = std::fstream(*this->dbFileName, std::ios::out | std::ios::binary);
	if (!this->writeStream.is_open()) {
		throw std::runtime_error("Failed to open db file for write: " + *this->dbFileName);
	}

	for (const auto& entry : this->data) {
		this->handleTransaction(StorageTransaction::Set, &entry.first, &entry.second);
	}
}

void LocalStorage::rebuild() {
	this->rebuildStorageSnapshot();
}

bool LocalStorage::hasItem(const std::string& key) const {
	return this->data.contains(key);
}

std::string LocalStorage::getItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	if (!this->data.contains(key)) return {};
	return this->data.find(key)->second;
}

void LocalStorage::setItem(const std::string& key, const std::string& value) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data[key] = value;
	handleTransaction(StorageTransaction::Set, &key, &value);
}

void LocalStorage::removeItem(const std::string& key) {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.erase(key);
	handleTransaction(StorageTransaction::Remove, &key, nullptr);
}

void LocalStorage::clear() {
	std::lock_guard <std::mutex> lock(this->mtlock);
	this->data.clear();
	handleTransaction(StorageTransaction::Clear, nullptr, nullptr);
}

size_t LocalStorage::length() const {
	return this->data.size();
}

size_t LocalStorage::size() const {
	return this->data.size();
}
