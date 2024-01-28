
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
				bool hasItem(const std::string& key) const noexcept;
				void setItem(const std::string& key, const std::string& value);
				void removeItem(const std::string& key);
				void clear();
				size_t size() const noexcept;
		};

		class KVDriver;
	};

	typedef WebStorage::KVInterface SessionStorage;

	class LocalStorage : public WebStorage::KVInterface {
		private:
			KVDriver* driver = nullptr;

		public:
			LocalStorage();
			LocalStorage(const std::string& dbfile);
			~LocalStorage();
	};
};

#endif
