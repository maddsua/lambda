#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>

#include "../http/http.hpp"

namespace Lambda {

	struct StaticFileInfo {
		size_t size;
		time_t modified;
		time_t created;
	};

	class StaticFile : public StaticFileInfo {
		private:
			//	todo: ptr to tar reader or fs stream

		public:
			std::string mime_type() const noexcept;
			std::vector<uint8_t> data();
			std::vector<uint8_t> data(size_t begin, size_t end);
	};

	class FsReader {
		public:
			virtual std::optional<StaticFile> open(const std::string& filename) = 0;
	};

	class StaticServer {
		private:
			std::string m_root;
			FsReader* m_reader_ptr = nullptr;

		public:
			StaticServer(const std::string& rootDir);
			~StaticServer();

			void serve(Request& req, ResponseWriter& wrt);
	};
};

#endif
