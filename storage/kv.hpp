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

	enum KVSnapshotCompress {
		KVSNAP_COMPRESS_NONE = 0,
		KVSNAP_COMPRESS_BR = 1,
	};

	typedef std::unordered_map<std::string, KVMapEntry> KVMap;

	class KV {
		private:
			KVMap data;
			std::mutex threadLock;

		public:
			void set(const std::string& key, const std::string& value);
			bool has(const std::string& key);
			KVMapEntry get(const std::string& key);
			std::string getValue(const std::string& key);
			bool move(const std::string& key, const std::string& newKey);
			bool del(const std::string& key);
			std::vector<KVEntry> entries();

			Lambda::Error exportSnapshot(const char* filepath, KVSnapshotCompress compression);
			Lambda::Error exportSnapshot(const char* filepath) {
				return exportSnapshot(filepath, KVSNAP_COMPRESS_NONE);
			}
			Lambda::Error importSnapshot(const char* filepath);
	};
};

#endif
