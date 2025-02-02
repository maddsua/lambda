#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

std::string trim_file_path(std::string filepath) {

	if (filepath.starts_with('/')) {
		filepath.erase(0, 1);
	}

	//	todo: resolve shit
	return filepath;
}

struct FsDirectoryFile : public ServedFile {
	private:
		std::fstream m_stream;
	
	public:
		FsDirectoryFile(std::fstream&& stream, const std::string& name, size_t size, time_t modified)
			: ServedFile (name, size, modified), m_stream(std::move(stream)) {};

		std::vector<uint8_t> content() {
			return this->content(0, this->size);
		}

		std::vector<uint8_t> content(size_t begin, size_t end) {

			if (!this->m_stream.is_open()) {
				return {};
			}

			if (begin < 0 || begin >= end) {
				begin = 0;
			}

			if (end > this->size || end < begin) {
				end = this->size;
			}

			if (end - begin < 1) {
				return {};
			}

			auto buff = std::vector<uint8_t>(end - begin);

			this->m_stream.seekg(begin);
			this->m_stream.read((char*)buff.data(), buff.size());

			if (buff.size() > static_cast<size_t>(this->m_stream.gcount())) {
				buff.resize(this->m_stream.gcount());
			}

			return buff;
		}
};

FsDirectoryServe::FsDirectoryServe(const std::string& root_dir) {

	this->m_root = root_dir;

	for (auto& rune : this->m_root) {
		if (rune == '\\') {
			rune = '/';
		}		
	}

	if (!this->m_root.ends_with('/')) {
		this->m_root.push_back('/');
	}

	//	first of all check if destination exists
	if (!std::filesystem::exists(this->m_root)) {
		throw std::runtime_error("FsStaticReader root path '" + this->m_root + "' doesn't exist");
	}

	//	check if we got a directory or a file there
	if (!std::filesystem::is_directory(this->m_root)) {
		throw std::runtime_error("StaticServer root path '" + this->m_root + "' cannot be served");
	}
}

std::unique_ptr<ServedFile> FsDirectoryServe::open(const std::string& filename) {

	auto resolved = this->m_root + trim_file_path(filename);
	if (!resolved.size()) {
		return nullptr;
	}

	if (!std::filesystem::exists(resolved)) {
		return nullptr;
	} else if (!std::filesystem::is_regular_file(resolved)) {
		return nullptr;
	}

	auto file_stream = std::fstream(resolved, std::ios::in | std::ios::binary);
	if (!file_stream.is_open()) {
		return {};
	}

	return std::unique_ptr<ServedFile>(new FsDirectoryFile(
		std::move(file_stream),
		resolved,
		std::filesystem::file_size(resolved),
		std::chrono::duration_cast<std::chrono::seconds>(
			std::filesystem::file_time_type::clock::to_sys(
				std::filesystem::last_write_time(resolved)
			).time_since_epoch()
		).count()
	));
}
