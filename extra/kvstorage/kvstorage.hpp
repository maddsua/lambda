
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE__

#include <string>
#include <unordered_map>
#include <mutex>

namespace Lambda::Storage {

	namespace KVStorage {

		class Driver;

		typedef std::unordered_map<std::string, std::string> Container;

		enum struct TransactionType : uint16_t {
			Put, Remove, Clear
		};

		struct Transaction {
			TransactionType type;
			const std::string* key = nullptr;
			const std::string* value = nullptr;
		};

		class StorageInterface {
			protected:
				Container data;
				std::mutex mtlock;
				Driver* driver = nullptr;

			public:
				StorageInterface();
				StorageInterface(const std::string& dbfile);
				~StorageInterface();

				std::string getItem(const std::string& key);
				bool hasItem(const std::string& key) const noexcept;
				void setItem(const std::string& key, const std::string& value);
				void removeItem(const std::string& key);
				void clear();
				size_t size() const noexcept;
		};
	};

	typedef KVStorage::StorageInterface LocalStorage;

};

#endif
