#include "../storage.hpp"
#include "../fs.hpp"
#include "../../core/encoding.hpp"

#include <stdexcept>
#include <iterator>
#include <filesystem>

using namespace Lambda;
using namespace Lambda::Storage;

enum DiskDbWriteOps {
	WriteOpStartBlock = 0x06,
	WriteOpFieldSep = 0x1f,
	WriteOpBlockEnd = 0x00,
	WriteOpSet = 0x01,
	WriteOpDel = 0x02,
	WriteOpDrop = 0x03,
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
	this->filestream.close();
	delete this->dbFileName;
	delete this->stats;
}

void LocalStorage::loadFile(const std::string& dbfile) {

	if (std::filesystem::exists(dbfile)) {
	
		auto rawcontent = FS::readFileSync(dbfile);

		std::vector<std::vector<uint8_t>> messages;

		if (rawcontent.size()) {
			size_t lastMessageStart = 0;
			for (size_t i = lastMessageStart; i < rawcontent.size(); i++) {
				if (rawcontent[i] == DiskDbWriteOps::WriteOpBlockEnd) {
					messages.push_back(std::vector<uint8_t>(rawcontent.begin() + lastMessageStart, rawcontent.begin() + i));
					lastMessageStart = i + 1;
				}
			}
		}

		for (size_t i = 0; i < messages.size(); i++) {

			const auto& entry = messages[i];

			if (entry.at(0) != DiskDbWriteOps::WriteOpStartBlock)
				throw std::runtime_error("Invalid start byte in message " + std::to_string(i));

			auto getFields = [&entry]() {
				std::vector<std::vector<uint8_t>> messageFields;
				size_t lastFieldStart = 2;
				for (size_t i = lastFieldStart; i < entry.size(); i++) {
					if (entry[i] == DiskDbWriteOps::WriteOpFieldSep) {
						messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.begin() + i));
						lastFieldStart = i + 1;
					}
				}
				messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.end()));
				return messageFields;
			};

			switch (entry.at(1)) {

				case DiskDbWriteOps::WriteOpSet: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					auto& value = fields.at(1);
					auto decodedValue = Encoding::fromBase64(std::string(value.begin(), value.end()));

					this->data[std::string(decodedKey.begin(), decodedKey.end())] = std::string(decodedValue.begin(), decodedValue.end());

				} break;

				case DiskDbWriteOps::WriteOpDel: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					this->data.erase(std::string(decodedKey.begin(), decodedKey.end()));

				} break;

				case DiskDbWriteOps::WriteOpDrop: {
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

		case StorageTransaction::Tr_Set: {

			if (!key) throw std::runtime_error("key pointer should be defined for write operation");
			if (!value) throw std::runtime_error("value pointer should be defined for write operation");

			writeBuff.insert(writeBuff.end(), {
				DiskDbWriteOps::WriteOpStartBlock,
				DiskDbWriteOps::WriteOpSet,
			});

			auto encodedKey = Encoding::toBase64(std::vector<uint8_t>(key->begin(), key->end()));
			writeBuff.insert(writeBuff.end(), encodedKey.begin(), encodedKey.end());

			writeBuff.insert(writeBuff.end(), { DiskDbWriteOps::WriteOpFieldSep });
			
			auto encodedValue = Encoding::toBase64(std::vector<uint8_t>(value->begin(), value->end()));
			writeBuff.insert(writeBuff.end(), encodedValue.begin(), encodedValue.end());
			
			writeBuff.insert(writeBuff.end(), { DiskDbWriteOps::WriteOpBlockEnd });

		} break;

		case StorageTransaction::Tr_Delete: {

			if (!key) throw std::runtime_error("key pointer should be defined for delete operation");

			writeBuff.insert(writeBuff.end(), {
				DiskDbWriteOps::WriteOpStartBlock,
				DiskDbWriteOps::WriteOpDel,
			});

			auto encodedKey = Encoding::toBase64(std::vector<uint8_t>(key->begin(), key->end()));
			writeBuff.insert(writeBuff.end(), encodedKey.begin(), encodedKey.end());

			writeBuff.insert(writeBuff.end(), { DiskDbWriteOps::WriteOpBlockEnd });

			if (this->stats != nullptr) this->stats->deletions++;

		} break;

		case StorageTransaction::Tr_Clear: {

			writeBuff.insert(writeBuff.end(), {
				DiskDbWriteOps::WriteOpStartBlock,
				DiskDbWriteOps::WriteOpDrop,
				DiskDbWriteOps::WriteOpBlockEnd
			});
			
		} break;

		default: throw std::runtime_error("unknown transaction type");
	}

	this->filestream.write((char*)writeBuff.data(), writeBuff.size());

	if (this->stats != nullptr && this->stats->deletions > 1000) {
		this->stats->deletions = 0;
		this->rebuildStorageSnapshot();
	}
}

void LocalStorage::rebuildStorageSnapshot() {

	if (this->dbFileName == nullptr) throw std::runtime_error("LocalStorage::dbFileName is undefined");

	if (this->filestream.is_open()) {
		this->filestream.close();
	}

	this->filestream = std::fstream(*this->dbFileName, std::ios::out | std::ios::binary);
	if (!this->filestream.is_open()) {
		throw std::runtime_error("Failed to open db file for write: " + *this->dbFileName);
	}

	for (const auto& entry : this->data) {
		this->handleTransaction(StorageTransaction::Tr_Set, &entry.first, &entry.second);
	}
}

void LocalStorage::rebuild() {
	this->rebuildStorageSnapshot();
}
