#ifndef __LAMBDA_FILESERVER__
#define __LAMBDA_FILESERVER__

#include <string>
#include <optional>
#include <vector>

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

			HTTP::Response serve(const HTTP::Request& request) const noexcept;
			HTTP::Response serve(const std::string& pathname) const noexcept;

			std::string flattenPathName(std::string urlpath) const noexcept;
			std::optional<std::string> resolvePath(const std::string& pathname) const noexcept;
	};	
};

#endif
