#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>

#include "../http/http.hpp"

namespace Lambda {

	const std::string& get_file_mimetype(const std::string& filename);

	struct StaticFileInfo {
		std::string resolved_path;
		size_t size;
		time_t modified;
	};

	class StaticFile : public StaticFileInfo {
		private:
			FsReader* m_reader_ptr = nullptr;

		public:
			StaticFile(FsReader* reader_ptr, StaticFile info);

			std::string mime_type() const noexcept;
			std::vector<uint8_t> content();
			std::vector<uint8_t> content(size_t begin, size_t end);
	};

	class FsReader {
		public:
			virtual std::optional<StaticFile> open(const std::string& filename) = 0;
			virtual std::vector<uint8_t> content(const StaticFileInfo& reader_file) = 0;
			virtual std::vector<uint8_t> content(const StaticFileInfo& reader_file, size_t begin, size_t end) = 0;

		friend class StaticFile;
	};

	class StaticReader : public FsReader {
		private:
			std::string m_root;

		public:
			StaticReader(const std::string& root_dir);

			std::optional<StaticFile> open(const std::string& filename);
			std::vector<uint8_t> content(const StaticFileInfo& reader_file);
			std::vector<uint8_t> content(const StaticFileInfo& reader_file, size_t begin, size_t end);
	};

	class StaticServer {
		private:
			//	todo: use unique pointer
			FsReader* m_reader_ptr = nullptr;

		public:
			StaticServer(const std::string& root_dir);
			~StaticServer();

			void serve(Request& req, ResponseWriter& wrt);
	};
};

#endif
