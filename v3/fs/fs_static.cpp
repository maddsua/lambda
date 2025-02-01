#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

std::string trim_file_path(const std::string& filepath) {
	//	todo: resolve shit
}

struct FsStaticFile : public FsServeFile {

	std::string mime_type() const noexcept {
		return Fs::infer_mimetype(this->name);
	}

	std::vector<uint8_t> content() {

		auto file = std::fstream(this->name, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			return {};
		}

		return std::vector<uint8_t>(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
	}

	std::vector<uint8_t> content(size_t begin, size_t end) {
	
		if (begin <= 0 || end < begin || !this->name.size()) {
			return {};
		}

		auto file = std::fstream(this->name, std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			return {};
		}

		auto file_size = file.tellg();
		if (end >= file_size) {
			return {};
		}

		file.seekg(begin);

		std::vector<uint8_t> data(end - begin);
		//	todo: replace with proper iterators
		file.read((char*)data.data(), data.size());

		if (data.size() != file.gcount()) {
			data.resize(file.gcount());
		}

		return data;
	}
};

class FsStaticReader : public FsServeReader {
	private:
		std::string m_root;

	public:

	FsStaticReader(const std::string& root_dir) {
		this->m_root = root_dir;
	}

	std::optional<FsStaticFile> open(const std::string& filename) {

		auto resolved = trim_file_path(filename);
		if (resolved.size()) {
			return std::nullopt;
		}

		if (!std::filesystem::exists(resolved)) {
			return std::nullopt;
		}

		FsStaticFile file;
		file.name = resolved;
		file.size = std::filesystem::file_size(resolved);
		file.modified = std::filesystem::last_write_time(resolved).time_since_epoch().count();

		return file;
	}
};
