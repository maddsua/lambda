#include "./vfs.hpp"
#include "../../core/polyfill/polyfill.hpp"
#include <filesystem>

using namespace Lambda;
using namespace Lambda::VFS;

std::string VirtualFile::text() const noexcept {
	return std::string(this->buffer.begin(), this->buffer.end());
};

size_t VirtualFile::size() const noexcept {
	return this->buffer.size();
};

const std::optional<VirtualFile> Interface::read(const std::string& path) noexcept {

	std::lock_guard<std::mutex> lock(this->m_lock);

	auto itr = this->m_data.find(path);
	if (itr == this->m_data.end()) return std::nullopt;

	VirtualFile vfsfile {
		itr->second.buffer,
		itr->second.created,
		itr->second.modified,
		itr->first
	};

	return vfsfile;
}

void Interface::write(const std::string& path, const std::vector <uint8_t>& content) noexcept {

	const auto pathNormalized = Strings::toLowerCase(path);

	std::lock_guard<std::mutex> lock(this->m_lock);

	const auto itr = this->m_data.find(pathNormalized);

	StoredValue vfsentry {
		content,
		itr != this->m_data.end() ? itr->second.created : std::time(nullptr),
		std::time(nullptr),
	};

	this->m_data[pathNormalized] = vfsentry;
}

void Interface::write(const std::string& path, const std::string& content) noexcept {
	this->write(path, std::vector <uint8_t>(content.begin(), content.end()));
}

void Interface::remove(const std::string& path) noexcept {

	const auto pathNormalized = Strings::toLowerCase(path);

	std::lock_guard<std::mutex> lock(this->m_lock);

	auto itr = this->m_data.find(pathNormalized);
	if (itr != this->m_data.end()) {
		this->m_data.erase(itr);
	}
}

void Interface::move(const std::string& path, const std::string& newPath) {

	const auto pathNormalized = Strings::toLowerCase(path);
	const auto newPathNormalized = Strings::toLowerCase(path);

	std::lock_guard <std::mutex> lock(this->m_lock);

	const auto itrOld = this->m_data.find(pathNormalized);
	if (itrOld == this->m_data.end()) {
		throw std::filesystem::filesystem_error("Virtual file not found", path, std::error_code(2L, std::generic_category()));
	}

	this->m_data[newPathNormalized] = itrOld->second;
	this->m_data.erase(itrOld);
}

std::vector<VirtualFileInfo> Interface::listAll() noexcept {

	std::vector<VirtualFileInfo> result;

	std::lock_guard<std::mutex> lock(this->m_lock);

	for (const auto& entry : this->m_data) {
		result.push_back({
			entry.first,
			entry.second.created,
			entry.second.modified,
			entry.second.buffer.size()
		});
	}

	return result;
}

std::optional<VirtualFileInfo> Interface::fileInfo(const std::string& path) noexcept {

	const auto pathNormalized = Strings::toLowerCase(path);

	std::lock_guard<std::mutex> lock(this->m_lock);

	auto itr = this->m_data.find(pathNormalized);
	if (itr == this->m_data.end()) {
		std::nullopt;
	}

	VirtualFileInfo info {
		itr->first,
		itr->second.created,
		itr->second.modified,
		itr->second.buffer.size()
	};

	return info;
}
