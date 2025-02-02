#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

std::string trim_file_path(std::string filepath);

FsStaticFile::FsStaticFile(const std::string& i_name, size_t i_size, time_t i_modified) {
	this->name = i_name;
	this->size = i_size;
	this->modified = i_modified;
}

std::vector<uint8_t> FsStaticFile::content() {

	auto file = std::fstream(this->name, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return {};
	}

	return std::vector<uint8_t>(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
}

std::vector<uint8_t> FsStaticFile::content(size_t begin, size_t end) {

	if (begin <= 0 || end < begin || !this->name.size()) {
		return {};
	}

	auto file = std::fstream(this->name, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return {};
	}

	size_t file_size = file.tellg();
	if (end >= file_size) {
		return {};
	}

	file.seekg(begin);

	std::vector<uint8_t> data(end - begin);
	file.read((char*)data.data(), data.size());

	if (data.size() != static_cast<size_t>(file.gcount())) {
		data.resize(file.gcount());
	}

	return data;
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

	//	this bullshit doesn't work apparantely
	return std::unique_ptr<FsServeFile>(new FsStaticFile(
		resolved,
		std::filesystem::file_size(resolved),
		//	todo: fix broken epoch
		std::filesystem::last_write_time(resolved).time_since_epoch() / std::chrono::nanoseconds(1)
	));
}

std::string trim_file_path(std::string filepath) {

	if (filepath.starts_with('/')) {
		filepath.erase(0, 1);
	}

	//	todo: resolve shit
	return filepath;
}
