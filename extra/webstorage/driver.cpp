
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

	/*if (!std::filesystem::exists(this->m_filename)) {

		auto readfile = std::ifstream(this->m_filename, std::ios::binary);
		if (!readfile.is_open()) {
			throw std::runtime_error("Could not open \"" + this->m_filename + "\" for read");
		}

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
				if (rawcontent[i] == DiskLogWriteOps::WriteOpBlockEnd) {
					messages.push_back(std::vector<uint8_t>(rawcontent.begin() + lastMessageStart, rawcontent.begin() + i));
					lastMessageStart = i + 1;
				}
			}
		}

		for (size_t i = 0; i < messages.size(); i++) {

			const auto& entry = messages[i];

			if (entry.at(0) != DiskLogWriteOps::WriteOpStartBlock)
				throw std::runtime_error("Invalid start byte in message " + std::to_string(i));

			auto getFields = [&entry]() {
				std::vector<std::vector<uint8_t>> messageFields;
				size_t lastFieldStart = 2;
				for (size_t i = lastFieldStart; i < entry.size(); i++) {
					if (entry[i] == DiskLogWriteOps::WriteOpFieldSep) {
						messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.begin() + i));
						lastFieldStart = i + 1;
					}
				}
				messageFields.push_back(std::vector<uint8_t>(entry.begin() + lastFieldStart, entry.end()));
				return messageFields;
			};

			switch (entry.at(1)) {

				case DiskLogWriteOps::WriteOpSet: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					auto& value = fields.at(1);
					auto decodedValue = Encoding::fromBase64(std::string(value.begin(), value.end()));

					this->data[std::string(decodedKey.begin(), decodedKey.end())] = std::string(decodedValue.begin(), decodedValue.end());

				} break;

				case DiskLogWriteOps::WriteOpDel: {

					auto fields = getFields();

					auto& key = fields.at(0);
					auto decodedKey = Encoding::fromBase64(std::string(key.begin(), key.end()));

					this->data.erase(std::string(decodedKey.begin(), decodedKey.end()));

				} break;

				case DiskLogWriteOps::WriteOpDrop: {
					this->data.clear();
				} break;

				default: throw std::runtime_error("Invalid action byte in message " + std::to_string(i));
			}
		}
	}*/

	this->m_stream = std::fstream(this->m_filename, std::ios::out | std::ios::binary);
	if (!this->m_stream.is_open()) {
		throw std::runtime_error("Failed to open db file for write: " + this->m_filename);
	}

	if (this->m_init_data != nullptr) {
		for (const auto& entry : *this->m_init_data) {
			this->handleTransaction({ TransactionType::Create, &entry.first, &entry.second });
		}
	}
}

KVDriver::~KVDriver() {
	this->m_stream.flush();
	this->m_stream.close();
	puts("destructing driver");
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

	KVDriver::RecordHeader record {
		static_cast<std::underlying_type_t<TransactionType>>(tractx.type),
		static_cast<uint16_t>(tractx.key ? tractx.key->size() : 0),
		static_cast<uint32_t>(tractx.value ? tractx.value->size() : 0)
	};

	this->m_stream.write((const char*)&record, sizeof(record));

	if (tractx.type != TransactionType::Clear) {

		if (tractx.key == nullptr) {
			throw std::runtime_error("a transaction should specify a key to operate on");
		}

		this->m_stream.write(tractx.key->data(), tractx.key->size());
	}

	if (tractx.type == TransactionType::Create || tractx.type == TransactionType::Update) {

		if (tractx.value == nullptr) {
			throw std::runtime_error("a write transaction should provida a value");
		}

		this->m_stream.write(tractx.value->data(), tractx.value->size());
	}

	this->m_stream.flush();
}
