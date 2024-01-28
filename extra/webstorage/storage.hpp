
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_STORAGE__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_STORAGE__

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <functional>
#include <optional>

namespace Lambda::Storage {

	namespace WebStorage {

		enum struct TransactionType {
			Create, Update, Remove, Clear
		};

		struct Transaction {
			TransactionType type;
			std::optional<const std::string&> key;
			std::optional<const std::string&> value;
		};

		typedef std::function<void(Transaction)> TransactionCallback;

		class KVInterface {
			protected:
				std::unordered_map<std::string, std::string> data;
				std::mutex mtlock;
				std::optional<WebStorage::TransactionCallback> callback;

			public:
				std::string getItem(const std::string& key);
				bool hasItem(const std::string& key) const;
				void setItem(const std::string& key, const std::string& value);
				void removeItem(const std::string& key);
				void clear();
				size_t length() const;
				size_t size() const;
		};
	};

	enum struct StorageTransaction {
		Remove = -1,
		Clear = 0,
		Set = 1,
	};

	struct StorageTrStats {
		size_t deletions = 0;
	};

	class SessionStorage {
		protected:
			std::unordered_map<std::string, std::string> data;
			std::mutex mtlock;
			std::optional<WebStorage::TransactionCallback> callback;

		public:
			std::string getItem(const std::string& key);
			bool hasItem(const std::string& key) const;
			void setItem(const std::string& key, const std::string& value);
			void removeItem(const std::string& key);
			void clear();
			size_t length() const;
			size_t size() const;
	};

	class LocalStorage {
		protected:
			std::string* dbFileName = nullptr;
			std::fstream writeStream;
			std::mutex mtlock;
			StorageTrStats* stats = nullptr;
			std::unordered_map<std::string, std::string> data;
			void loadFile(const std::string& dbfile);
			void handleTransaction(StorageTransaction tra, const std::string* key, const std::string* value);
			void rebuildStorageSnapshot();

		public:
			LocalStorage();
			LocalStorage(const std::string& dbfile);
			~LocalStorage();
			void rebuild();

			std::string getItem(const std::string& key);
			bool hasItem(const std::string& key) const;
			void setItem(const std::string& key, const std::string& value);
			void removeItem(const std::string& key);
			void clear();
			size_t length() const;
			size_t size() const;
	};
};

#endif
