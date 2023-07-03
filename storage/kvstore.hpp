#ifndef __LAMBDA_STORE_KV__
#define __LAMBDA_STORE_KV__

#include "../lambda.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

namespace Lambda::Storage {

	struct KVMapEntry {
		std::string value;
		time_t created = 0;
		time_t modified = 0;
	};

	struct KVEntry : public KVMapEntry {
		std::string key;
	};

	class KV {
		private:
			std::unordered_map<std::string, KVMapEntry> data;
			std::mutex threadLock;

		public:
			void set(const std::string& key, const std::string& value);
			bool has(const std::string& key);
			KVMapEntry get(const std::string& key);
			std::string getValue(const std::string& key);
			bool move(const std::string& key, const std::string& newKey);
			bool del(const std::string& key);
			std::vector<KVEntry> entries();

			Lambda::Error exportStore(const char* filepath);
			Lambda::Error importStore(const char* filepath);
	};
};

#endif
