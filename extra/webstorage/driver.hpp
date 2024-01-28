
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__

#include "./interface.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <mutex>
#include <fstream>

namespace Lambda::Storage::WebStorage {

	class KVDriver {
		public:
			KVDriver(const std::string& filename);
			~KVDriver();
			void handleTransaction(const Transaction&);
			std::unordered_map<std::string, std::string> load();
	};

};

#endif
