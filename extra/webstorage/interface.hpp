
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE__

#include <string>
#include <unordered_map>
#include <mutex>

namespace Lambda::Storage {

	namespace WebStorage {

		class KVDriver;

		enum struct TransactionType {
			Create, Update, Remove, Clear
		};

		struct Transaction {
			TransactionType type;
			const std::string* key = nullptr;
			const std::string* value = nullptr;
		};

		class KVInterface {
			protected:
				std::unordered_map<std::string, std::string> data;
				std::mutex mtlock;
				KVDriver* driver = nullptr;

			public:
				std::string getItem(const std::string& key);
				bool hasItem(const std::string& key) const noexcept;
				void setItem(const std::string& key, const std::string& value);
				void removeItem(const std::string& key);
				void clear();
				size_t size() const noexcept;
		};
	};

	typedef WebStorage::KVInterface SessionStorage;

	class LocalStorage : public WebStorage::KVInterface {
		public:
			LocalStorage();
			LocalStorage(const std::string& dbfile);
			~LocalStorage();
	};
};

#endif
