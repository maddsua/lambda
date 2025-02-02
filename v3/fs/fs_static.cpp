#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

std::string trim_file_path(std::string filepath);

std::vector<uint8_t> FsStaticFile::content() {
	return this->content(0, this->size);
}

std::vector<uint8_t> FsStaticFile::content(size_t begin, size_t end) {

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

FsStaticReader::FsStaticReader(const std::string& root_dir) {

	this->m_root = root_dir;

	//	todo: also flip slashes on windows

	if (!this->m_root.ends_with('/')) {
		this->m_root.push_back('/');
	}
}

std::unique_ptr<FsServeFile> FsStaticReader::open(const std::string& filename) {

	auto resolved = this->m_root + trim_file_path(filename);
	if (!resolved.size()) {
		return nullptr;
	}

	if (!std::filesystem::exists(resolved)) {
		return nullptr;
	} else if (!std::filesystem::is_regular_file(resolved)) {
		return nullptr;
	}

	auto file_size = std::filesystem::file_size(resolved);

	auto file_stream = std::fstream(resolved, std::ios::in | std::ios::binary);
	if (!file_stream.is_open()) {
		return {};
	}

	//	todo: fix broken epoch
	auto file_modified = std::filesystem::last_write_time(resolved).time_since_epoch() / std::chrono::nanoseconds(1);

	return std::unique_ptr<FsServeFile>(new FsStaticFile(
		std::move(file_stream),
		resolved,
		file_size,
		file_modified
	));
}

std::string trim_file_path(std::string filepath) {

	if (filepath.starts_with('/')) {
		filepath.erase(0, 1);
	}

	//	todo: resolve shit
	return filepath;
}
