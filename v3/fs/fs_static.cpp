#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

std::string resolve_static_file_path(const std::string& filepath);

StaticReader::StaticReader(const std::string& root_dir) {
	this->m_root = root_dir;
}

std::vector<uint8_t> StaticReader::content(const StaticFileInfo& reader_file) {

	auto file = std::fstream(reader_file.resolved_path, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return {};
	}

	return std::vector<uint8_t>(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
}

std::vector<uint8_t> StaticReader::content(const StaticFileInfo& reader_file, size_t begin, size_t end) {

	if (begin <= 0 || end < begin || !reader_file.resolved_path.size()) {
		return {};
	}

	auto file = std::fstream(reader_file.resolved_path, std::ios::in | std::ios::binary | std::ios::ate);
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

std::optional<StaticFile> StaticReader::open(const std::string& filepath) {

	auto resolved = resolve_static_file_path(filepath);
	if (resolved.size()) {
		return std::nullopt;
	}

	if (!std::filesystem::exists(resolved)) {
		return std::nullopt;
	}

	StaticFileInfo info {
		.resolved_path = resolved,
		.size = std::filesystem::file_size(resolved),
		.modified = std::filesystem::last_write_time(resolved).time_since_epoch().count(),
	};

	return StaticFile(this, info);
}

std::string resolve_static_file_path(const std::string& filepath) {
	//	todo: resolve shit
}
