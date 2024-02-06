
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_WEB_STORAGE_DRIVER__

#include "./kvstorage.hpp"

#include <fstream>
#include <optional>

namespace Lambda::Storage::KVStorage {

	class Driver {
		private:
			constexpr static char const magicstring[] = "mlkvdb";

			const std::string m_filename;
			std::fstream m_stream;
			Container* m_init_data = nullptr;

			struct DBBasicHeader {
				char magic[sizeof(magicstring) - 1];
				uint16_t version;
			};

			struct RecordHeader {
				uint16_t type;
				uint16_t keySize;
				uint32_t valueSize;
			};

		public:
			Driver(const std::string& filename);
			~Driver();
			void handleTransaction(const Transaction& tractx);
			std::optional<Container> sync();

			static const uint32_t version = 3;
	};
};

#endif
