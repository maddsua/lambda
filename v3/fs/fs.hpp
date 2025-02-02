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
	struct FsServeFile {
		FsServeFile(std::string name, size_t size, time_t modified)
			: name(name), size(size), modified(modified) {}

		std::string name;
		size_t size = 0;
		time_t modified = 0;

		virtual std::vector<uint8_t> content() = 0;
		virtual std::vector<uint8_t> content(size_t begin, size_t end) = 0;
	};

	class FsServeReader {
		public:
			virtual std::unique_ptr<FsServeFile> open(const std::string& filename) = 0;	
	};

	struct FsStaticFile : public FsServeFile {
		private:
			std::fstream m_stream;
		
		public:
			FsStaticFile(std::fstream&& stream, const std::string& name, size_t size, time_t modified)
				: FsServeFile (name, size, modified), m_stream(std::move(stream)) {};

			std::vector<uint8_t> content();
			std::vector<uint8_t> content(size_t begin, size_t end);
	};

	class FsStaticReader : public FsServeReader {
		private:
			std::string m_root;

		public:
			FsStaticReader(const std::string& root_dir);
			std::unique_ptr<FsServeFile> open(const std::string& filename);
	};

	class StaticServer {
		private:
			std::unique_ptr<FsServeReader> m_reader_ptr;

		public:
			StaticServer(const std::string& root);

			void handle(Request& req, ResponseWriter& wrt);
			HandlerFn handler() noexcept;
	};

	namespace Fs {
		const std::string& infer_mimetype(const std::string& filename);
	};
};

#endif
