
#include "./driver.hpp"
#include "./kvstorage.hpp"
#include "../../core/utils/utils.hpp"

#include <filesystem>
#include <vector>
#include <iterator>
#include <memory.h>

using namespace Lambda::Storage;
using namespace Lambda::Storage::KVStorage;

Driver::Driver(const std::string& filename) : m_filename(filename) {

	if (std::filesystem::exists(this->m_filename)) {

		this->m_stream = std::fstream(this->m_filename, std::ios::in | std::ios::binary);
		if (!this->m_stream.is_open()) {
			throw std::runtime_error("Could not open \"" + this->m_filename + "\" for read");
		}

		DBBasicHeader dbHeader;
		this->m_stream.read((char*)&dbHeader, sizeof(dbHeader));
		dbHeader.version = Bits::storenormx(dbHeader.version);

		//	check if a file is even our db
		if (memcmp(dbHeader.magic, this->magicstring, sizeof(dbHeader.magic))) {
			throw std::runtime_error("File \"" + this->m_filename + "\" is not recognized as a db file");
		}

		if (this->m_stream.gcount() != sizeof(dbHeader)) {
			throw std::runtime_error("Corrupt db file: cound not read db version");
		}

		//	replace it later with proper version handling
		if (dbHeader.version != this->version) {
			throw std::runtime_error("Unsupported db version");
		}

		this->m_init_data = new Container();

		while (this->m_stream.is_open() && !this->m_stream.eof()) {

			RecordHeader recordHeader;
			this->m_stream.read((char*)&recordHeader, sizeof(recordHeader));
			auto lastRead = this->m_stream.gcount();
			
			if (lastRead != sizeof(recordHeader)) {
				if (!lastRead) break;
				throw std::runtime_error("Corrupt db file: incomplete record header");
			}

			auto opationType = static_cast<TransactionType>(Bits::storenormx(recordHeader.type));
			recordHeader.keySize = Bits::storenormx(recordHeader.keySize);
			recordHeader.valueSize = Bits::storenormx(recordHeader.valueSize);

			switch (opationType) {

				case TransactionType::Put: {

					std::string putKey;
					putKey.resize(recordHeader.keySize);

					this->m_stream.read(putKey.data(), putKey.size());
					if (this->m_stream.gcount() != recordHeader.keySize) {
						throw std::runtime_error("Corrupt db file: incomplete record data");
					}

					std::string putValue;
					putValue.resize(recordHeader.valueSize);

					this->m_stream.read(putValue.data(), putValue.size());
					if (this->m_stream.gcount() != recordHeader.valueSize) {
						throw std::runtime_error("Corrupt db file: incomplete record data");
					}
					
					(*this->m_init_data)[putKey] = putValue;

				} break;

				case TransactionType::Remove: {

					if (recordHeader.valueSize) {
						throw std::runtime_error("Corrupt db file: Remove transaction cannot carry a record value");
					}

					std::string removeKey;
					removeKey.resize(recordHeader.keySize);

					this->m_stream.read(removeKey.data(), removeKey.size());
					if (this->m_stream.gcount() != recordHeader.keySize) {
						throw std::runtime_error("Corrupt db file: incomplete record data");
					}

					if (this->m_init_data->contains(removeKey)) {
						this->m_init_data->erase(removeKey);
					}
					
				} break;

				case TransactionType::Clear: {

					if (recordHeader.keySize) {
						throw std::runtime_error("Corrupt db file: Clear transaction cannot carry a record key");
					}

					if (recordHeader.valueSize) {
						throw std::runtime_error("Corrupt db file: Clear transaction cannot carry a record value");
					}

					this->m_init_data->clear();
					
				} break;

				default:
					throw std::runtime_error("Corrupt db file: unknown TransactionType (" + std::to_string(recordHeader.type) + ")");
			}
		}
	}

	auto dbfileParentDir = std::filesystem::path(this->m_filename).remove_filename();
	if (!std::filesystem::exists(dbfileParentDir)) {
		std::filesystem::create_directories(dbfileParentDir);
	}

	this->m_stream = std::fstream(this->m_filename, std::ios::out | std::ios::binary);
	if (!this->m_stream.is_open()) {
		throw std::runtime_error("Failed to open db file for write: " + this->m_filename);
	}

	DBBasicHeader dbHeader;
	memcpy(dbHeader.magic, this->magicstring, sizeof(dbHeader.magic));
	dbHeader.version = Bits::storenormx(this->version);

	this->m_stream.write((const char*)&dbHeader, sizeof(dbHeader));

	if (this->m_init_data != nullptr) {
		for (const auto& entry : *this->m_init_data) {
			this->handleTransaction({ TransactionType::Put, &entry.first, &entry.second });
		}
	}
}

Driver::~Driver() {

	this->m_stream.flush();
	this->m_stream.close();
	
	if (this->m_init_data) {
		delete this->m_init_data;
	}
}

std::optional<Container> Driver::sync() {

	if (this->m_init_data == nullptr) {
		return std::nullopt;
	}

	auto temp = std::move(*this->m_init_data);

	delete this->m_init_data;
	this->m_init_data = nullptr;

	return temp;
}

void Driver::handleTransaction(const Transaction& tractx) {

	Driver::RecordHeader recordHeader {
		Bits::storenormx(static_cast<std::underlying_type_t<TransactionType>>(tractx.type)),
		Bits::storenormx(static_cast<uint16_t>(tractx.key ? tractx.key->size() : 0)),
		Bits::storenormx(static_cast<uint32_t>(tractx.value ? tractx.value->size() : 0))
	};

	this->m_stream.write((const char*)&recordHeader, sizeof(recordHeader));

	if (tractx.type != TransactionType::Clear) {

		if (tractx.key == nullptr) {
			throw std::runtime_error("a transaction should specify a key to operate on");
		}

		this->m_stream.write(tractx.key->data(), tractx.key->size());
	}

	if (tractx.type == TransactionType::Put) {

		if (tractx.value == nullptr) {
			throw std::runtime_error("a write transaction should provida a value");
		}

		this->m_stream.write(tractx.value->data(), tractx.value->size());
	}

	this->m_stream.flush();
}
