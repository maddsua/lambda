#ifndef __LAMBDA_STORE_KV__
#define __LAMBDA_STORE_KV__

#include "../lambda.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

namespace Lambda::Storage {

	struct KVEntry {
		std::string value;
		time_t created = 0;
		time_t modified = 0;
	};

	struct KVStoreHeader {
		uint16_t version;
		uint8_t compression;
	};

	struct KVRecordHeader {
		uint8_t type;
		uint64_t created;
		uint64_t modified;
		uint16_t keySize;
		uint16_t valueSize;
	};

	enum KVEntryTypes {
		KVENTRY_DATA = 0,
		KVENTRY_META = 1,
	};

	enum KVStoreCompression {
		KVCOMPRESS_NONE = 0,
		KVCOMPRESS_BR_CHUNK = 1,
	};

	class KV {
		private:
			std::unordered_map<std::string, KVEntry> data;
			std::mutex threadLock;

		public:
			void set(const std::string& key, const std::string& value);
			bool has(const std::string& key);
			KVEntry get(const std::string& key);
			std::string getValue(const std::string& key);
			bool move(const std::string& key, const std::string& newKey);
			bool del(const std::string& key);

			Lambda::Error exportBundle(const char* filepath, uint8_t compression);
			Lambda::Error exportStore(const char* filepath) {
				return exportBundle(filepath, KVCOMPRESS_NONE);
			};

			Lambda::Error importStore(const char* filepath);
	};
};

#endif
