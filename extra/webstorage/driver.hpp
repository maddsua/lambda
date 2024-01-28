
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__

#include "./interface.hpp"

#include <string>
#include <unordered_map>
#include <fstream>

namespace Lambda::Storage::WebStorage {

	class KVDriver {
		private:
			const std::string m_filename;
			std::fstream m_stream;
			std::unordered_map<std::string, std::string>* m_init_data = nullptr;

		public:
			KVDriver(const std::string& filename);
			~KVDriver();
			void handleTransaction(const Transaction&);
			std::unordered_map<std::string, std::string> sync();
	};

};

#endif
