
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

			struct RecordHeader {
				uint8_t type;
				uint16_t keySize = 0;
				uint32_t valueSize = 0;
			};

		public:
			KVDriver(const std::string& filename);
			~KVDriver();
			void handleTransaction(const Transaction& tractx);
			std::unordered_map<std::string, std::string> sync();
	};

};

#endif
