#ifndef __LAMBDA_DB_KV__
#define __LAMBDA_DB_KV__

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

	typedef std::unordered_map<std::string, KVEntry> KVMap;
	
	class KV {
		private:
			KVMap data;
			std::mutex threadLock;

		public:
			KV();
			~KV();

			void set(const std::string key, const std::string& value);
			bool has(const std::string key);
			KVEntry get(std::string key);
			bool move(const std::string key, std::string newKey);
			bool del(const std::string key);
	};
};

#endif
