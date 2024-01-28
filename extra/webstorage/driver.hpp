
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__

#include "./interface.hpp"

#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <functional>
#include <optional>

namespace Lambda::Storage::WebStorage {

	class KVDriver {
		public:
			KVDriver(const std::string& filename);
			~KVDriver();
			void handleTransaction(const Transaction&);
	};

};

#endif
