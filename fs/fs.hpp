#ifndef __LIB_MADDSUA_LAMBDA_FILESERVER__
#define __LIB_MADDSUA_LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>

#include "../http/http.hpp"

namespace Lambda {

	class ServedFile {
		public:

			enum struct Type {
				File,
				Directory
			};

			virtual std::string name() const = 0;
			virtual Type type() const noexcept = 0;
			virtual time_t modified() = 0;
			virtual size_t size() = 0;

			virtual std::vector<uint8_t> content() = 0;
			virtual std::vector<uint8_t> content(size_t begin, size_t end) = 0;

			virtual ~ServedFile() = default;
	};

	class FileServerReader {
		public:
			virtual std::unique_ptr<ServedFile> open(const std::string& filename) = 0;	
	};

	class FsDirectoryServe : public FileServerReader {
		private:
			std::filesystem::path m_root;

		public:
			FsDirectoryServe(const std::string& root_dir);
			std::unique_ptr<ServedFile> open(const std::string& filename);
	};

	struct FileServerOptions {
		bool html_error_pages = true;
		bool debug = false;
	};

	//	todo: fix handler method abstraction

	class FileServer {
		private:
			//	todo: replace with unique_ptr
			FileServerReader& m_reader;

		public:
			FileServer(FileServerReader& reader) : m_reader(reader) {}
			FileServer(FileServerReader& reader, FileServerOptions options) : m_reader(reader), opts(options) {}

			FileServerOptions opts;

			void handle(Request& req, ResponseWriter& wrt);
			HandlerFn handler_fn();
	};

	namespace Fs {
		const std::string& infer_mimetype(const std::string& filename);
	};
};

#endif
