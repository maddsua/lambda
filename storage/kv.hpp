#ifndef __LAMBDA_STORE_KV__
#define __LAMBDA_STORE_KV__

#include "../lambda_private.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

#define LAMBDAKV_VERSION 	(1)

namespace Lambda::Storage {

	struct KVMapEntry {
		std::string value;
		time_t created = 0;
		time_t modified = 0;
	};

	struct KVEntry : public KVMapEntry {
		std::string key;
	};

	/**
	 * Snapshot binary representation compression options
	*/
	enum KVSnapshotCompress {
		KVSNAP_COMPRESS_NONE = 0,
		KVSNAP_COMPRESS_BR = 1,
	};

	typedef std::unordered_map<std::string, KVMapEntry> KVMap;

	/**
	 * Key-Value storage class
	*/
	class KV {
		private:
			KVMap data;
			std::mutex threadLock;

		public:
			/**
			 * Store data in KV. The data does not have to be a string only, binary data is acceptable too.
			 * 
			 * Key size is limited to UINT16_MAX (65535, 64Kb).
			 * 
			 * Value size is limited to UINT32_MAX (4294967295, 4GB).
			*/
			void set(const std::string& key, const std::string& value);

			/**
			 * Check if key is present in this KV storage instanse
			*/
			bool has(const std::string& key);

			/**
			 * Retrieves a value along with it's metadata
			*/
			KVMapEntry get(const std::string& key);

			/**
			 * Retrieves a value
			*/
			std::string getValue(const std::string& key);

			/**
			 * Rename a data record (change key)
			*/
			bool move(const std::string& key, const std::string& newKey);

			/**
			 * Deletes data record
			*/
			bool del(const std::string& key);

			/**
			 * List all data records
			*/
			std::vector<KVEntry> entries();

			/**
			 * Save KV storage snapshot to a binary representation
			*/
			Lambda::Error exportSnapshot(const char* filepath, KVSnapshotCompress compression);
			Lambda::Error exportSnapshot(const char* filepath) {
				return exportSnapshot(filepath, KVSNAP_COMPRESS_NONE);
			}

			/**
			 * Load KV snapshot from binary representation
			*/
			Lambda::Error importSnapshot(const char* filepath);

			/**
			 * Save KV storage snapshot to JSON
			*/
			Lambda::Error exportJSON(const char* filepath);

			/**
			 * Load KV snapshot from JSON
			*/
			Lambda::Error importJSON(const char* filepath);
	};
};

#endif
