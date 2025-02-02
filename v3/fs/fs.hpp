#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

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

	class FileServer {
		private:
			FileServerReader& m_reader;
			void handle_request(Request& req, ResponseWriter& wrt);

		public:
			bool html_error_pages = true;
			bool debug = false;

			FileServer(FileServerReader& reader) : m_reader(reader) {}

			//	todo: fix lifetime
			HandlerFn handler() noexcept;
	};

	namespace Fs {
		const std::string& infer_mimetype(const std::string& filename);
	};
};

#endif
