#include "./fs.hpp"

#include <iterator>
#include <fstream>
#include <filesystem>

using namespace Lambda;

struct FsDirectoryFile : public ServedFile {
	private:
		std::unique_ptr<std::fstream> m_stream;
		std::string m_resolved_Path;
		std::optional<time_t> m_mod;
		std::optional<size_t> m_size;

	public:
		FsDirectoryFile(std::fstream* stream, const std::string& path)
			: m_stream(std::unique_ptr<std::fstream>(stream)), m_resolved_Path(path) {};

		std::string name() const {
			return this->m_resolved_Path;
		}

		time_t modified() {

			if (!this->m_stream) {
				return 0;
			}

			if (this->m_mod.has_value()) {
				return this->m_mod.value();
			}

			this->m_mod = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(
				std::filesystem::file_time_type::clock::to_sys(
					std::filesystem::last_write_time(this->m_resolved_Path)
				).time_since_epoch()
			).count());

			return this->m_mod.value();
		}

		size_t size() {

			if (!this->m_stream) {
				return 0;
			}

			if (this->m_size.has_value()) {
				return this->m_size.value();
			}

			this->m_size = static_cast<size_t>(std::filesystem::file_size(this->m_resolved_Path));

			return this->m_size.value();
		}

		Type type() const noexcept {
			return this->m_stream ? ServedFile::Type::File : ServedFile::Type::Directory;
		}

		std::vector<uint8_t> content() {
			return this->content(0, this->size());
		}

		std::vector<uint8_t> content(size_t begin, size_t end) {

			if (!this->m_stream) {
				return {};
			}

			if (!this->m_stream->is_open()) {
				return {};
			}

			if (begin < 0 || begin >= end) {
				begin = 0;
			}

			if (end > this->size() || end < begin) {
				end = this->size();
			}

			if (end - begin < 1) {
				return {};
			}

			auto buff = std::vector<uint8_t>(end - begin);

			this->m_stream->seekg(begin);
			this->m_stream->read((char*)buff.data(), buff.size());

			if (buff.size() > static_cast<size_t>(this->m_stream->gcount())) {
				buff.resize(this->m_stream->gcount());
			}

			return buff;
		}
};

FsDirectoryServe::FsDirectoryServe(const std::string& root_dir) {

	this->m_root = std::filesystem::path(root_dir);
	if (this->m_root.empty()) {
		this->m_root = std::filesystem::path("./");
	}

	//	first of all check if destination exists
	if (!std::filesystem::exists(this->m_root)) {
		throw std::runtime_error("FsDirectoryServe root path '" + this->m_root.string() + "' doesn't exist");
	}

	//	check if we got a directory or a file there
	if (!std::filesystem::is_directory(this->m_root)) {
		throw std::runtime_error("FsDirectoryServe root path '" + this->m_root.string() + "' cannot be served");
	}
}

std::unique_ptr<ServedFile> FsDirectoryServe::open(const std::string& filename) {

	auto file_path = std::filesystem::path(filename).lexically_normal().string();
	auto path_absolute = file_path.starts_with('/') || file_path.starts_with('\\');
	auto resolved = this->m_root / std::filesystem::path(path_absolute ? file_path.substr(1) : file_path);

	if (std::filesystem::is_regular_file(resolved)) {
		return std::unique_ptr<ServedFile>(new FsDirectoryFile(
			new std::fstream(resolved, std::ios::in | std::ios::binary),
			resolved
		));
	}

	if (std::filesystem::is_directory(resolved)) {
		return std::unique_ptr<ServedFile>(new FsDirectoryFile(nullptr, resolved));
	}

	return nullptr;
}
