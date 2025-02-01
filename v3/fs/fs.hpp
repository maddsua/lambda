#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>

#include "../http/http.hpp"

namespace Lambda {

	struct FsServeFile {
		std::string name;
		size_t size;
		time_t modified;

		virtual std::string mime_type() const noexcept = 0;
		virtual std::vector<uint8_t> content() = 0;
		virtual std::vector<uint8_t> content(size_t begin, size_t end) = 0;
	};

	class FsServeReader {
		public:
			virtual std::optional<FsServeFile> open(const std::string& filename) = 0;	
	};

	class StaticServer {
		private:
			//	todo: use unique pointer
			FsServeFile* m_reader_ptr = nullptr;

		public:
			StaticServer(const std::string& root_dir);
			~StaticServer();

			void serve(Request& req, ResponseWriter& wrt);
	};

	namespace Fs {
		const std::string& infer_mimetype(const std::string& filename);
	};
};

#endif
