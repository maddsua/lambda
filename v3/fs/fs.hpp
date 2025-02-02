#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <fstream>

#include "../http/http.hpp"

namespace Lambda {

	//	todo: add caching/etag support
	//	todo: return file type from reader
	struct ServedFile {
		ServedFile(std::string name, size_t size, time_t modified)
			: name(name), size(size), modified(modified) {}

		std::string name;
		size_t size = 0;
		time_t modified = 0;

		virtual std::vector<uint8_t> content() = 0;
		virtual std::vector<uint8_t> content(size_t begin, size_t end) = 0;
	};

	class FileServerReader {
		public:
			virtual std::unique_ptr<ServedFile> open(const std::string& filename) = 0;	
	};

	class FsDirectoryServe : public FileServerReader {
		private:
			std::string m_root;

		public:
			FsDirectoryServe(const std::string& root_dir);
			std::unique_ptr<ServedFile> open(const std::string& filename);
	};

	class FileServer {
		private:
			std::unique_ptr<FileServerReader> m_reader;
			void handle_request(Request& req, ResponseWriter& wrt);

		public:
			FileServer(FileServerReader* reader);

			//	todo: fix lifetime
			HandlerFn handler() noexcept;
	};

	namespace Fs {
		const std::string& infer_mimetype(const std::string& filename);
	};
};

#endif
