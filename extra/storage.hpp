
#ifndef _LAMBDA_EXTRA_STORAGE_
#define _LAMBDA_EXTRA_STORAGE_

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <mutex>
#include <fstream>

/**
 * Storage object message
 *  ______________________
 * | [ACTION] SET         |
 * |----------------------|
 * | [KEY] cmVjb3JkX2lk   |
 * |----------------------|
 * | [VALUE] dGVzdCAxMjM  |
 * |----------------------|
 * | [EMPTY LINE]         |
 * |______________________|
*/

namespace Lambda::Storage {

	enum StorageTransaction {
		Tr_Delete = -1,
		Tr_Clear = 0,
		Tr_Set = 1,
	};

	struct StorageTrStats {
		size_t deletions = 0;
	};

	class BaseStorage {
		protected:
			std::unordered_map<std::string, std::string> data;
			std::mutex mtlock;
			virtual void handleTransaction(StorageTransaction tra, const std::string* key, const std::string* value) {}
			virtual void rebuildStorageSnapshot() {}
			StorageTrStats* stats = nullptr;
			std::string* dbFileName = nullptr;

		public:
			std::string getItem(const std::string& key);
			bool hasItem(const std::string& key) const;
			void setItem(const std::string& key, const std::string& value);
			void removeItem(const std::string& key);
			void clear();
			size_t length() const;
			size_t size() const;
	};

	typedef BaseStorage SessionStorage;

	class LocalStorage : public BaseStorage {
		protected:
			std::fstream filestream;
			void loadFile(const std::string& dbfile);
			void handleTransaction(StorageTransaction tra, const std::string* key, const std::string* value);
			void rebuildStorageSnapshot();

		public:
			LocalStorage();
			LocalStorage(const std::string& dbfile);
			~LocalStorage();
			void rebuild();
	};
};

#endif
