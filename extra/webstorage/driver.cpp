
#include "./interface.hpp"
#include "./driver.hpp"

#include <filesystem>
#include <vector>
#include <iterator>

using namespace Lambda::Storage;
using namespace Lambda::Storage::WebStorage;

enum DiskLogWriteOps {
	WriteOpStartBlock = 0x06,
	WriteOpFieldSep = 0x1f,
	WriteOpBlockEnd = 0x00,
	WriteOpSet = 0x01,
	WriteOpDel = 0x02,
	WriteOpDrop = 0x03,
};

KVDriver::KVDriver(const std::string& filename) : m_filename(filename) {

	puts("creating driver");

	if (std::filesystem::exists(this->m_filename)) {

		this->m_stream = std::fstream(this->m_filename, std::ios::in | std::ios::binary);
		if (!this->m_stream.is_open()) {
			throw std::runtime_error("Could not open \"" + this->m_filename + "\" for read");
		}

		this->m_init_data = new KVStorage();

		while (this->m_stream.is_open() && !this->m_stream.eof()) {

			KVDriver::RecordHeader recordHeader;
			this->m_stream.read((char*)&recordHeader, sizeof(recordHeader));
			
			if (this->m_stream.gcount() != sizeof(recordHeader)) {
				throw std::runtime_error("Corrupt db file: incomplete record header");
			}

			auto opType = static_cast<TransactionType>(recordHeader.type);

			switch (opType) {

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

					this->m_init_data->erase(removeKey);
					
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

	this->m_stream = std::fstream(this->m_filename, std::ios::out | std::ios::binary);
	if (!this->m_stream.is_open()) {
		throw std::runtime_error("Failed to open db file for write: " + this->m_filename);
	}

	if (this->m_init_data != nullptr) {
		for (const auto& entry : *this->m_init_data) {
			this->handleTransaction({ TransactionType::Put, &entry.first, &entry.second });
		}
	}
}

KVDriver::~KVDriver() {

	this->m_stream.flush();
	this->m_stream.close();
	
	if (this->m_init_data) {
		delete this->m_init_data;
	}
}

std::optional<KVStorage> KVDriver::sync() {

	if (this->m_init_data == nullptr) {
		return std::nullopt;
	}

	auto temp = std::move(*this->m_init_data);

	delete this->m_init_data;
	this->m_init_data = nullptr;

	return temp;
}

void KVDriver::handleTransaction(const Transaction& tractx) {

	KVDriver::RecordHeader recordHeader {
		static_cast<std::underlying_type_t<TransactionType>>(tractx.type),
		static_cast<uint16_t>(tractx.key ? tractx.key->size() : 0),
		static_cast<uint32_t>(tractx.value ? tractx.value->size() : 0)
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
