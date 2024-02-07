#include "./vfs.hpp"
#include "../../core/polyfill/polyfill.hpp"
#include "./formats.hpp"

#include <filesystem>
#include <future>

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
	const auto newSize = path.size() + content.size();

	std::lock_guard<std::mutex> lock(this->m_lock);

	const auto itr = this->m_data.find(pathNormalized);

	if (itr == this->m_data.end()) {
		this->m_info.totalSize += newSize;
		this->m_info.totalFiles++;
	} else {
		const auto oldSize = itr->first.size() + itr->second.buffer.size();
		this->m_info.totalSize += (newSize - oldSize);
	}

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
		this->m_info.totalSize -= (itr->first.size() + itr->second.buffer.size());
		this->m_info.totalFiles--;
		this->m_data.erase(itr);
	}
}

void Interface::move(const std::string& oldPath, const std::string& newPath) {

	const auto pathNormalized = Strings::toLowerCase(oldPath);
	const auto newPathNormalized = Strings::toLowerCase(newPath);

	this->m_info.totalSize += (newPath.size() - oldPath.size());
	
	std::lock_guard <std::mutex> lock(this->m_lock);

	const auto itrOld = this->m_data.find(pathNormalized);
	if (itrOld == this->m_data.end()) {
		throw std::filesystem::filesystem_error("Virtual file not found", oldPath, std::error_code(2L, std::generic_category()));
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
		return std::nullopt;
	}

	VirtualFileInfo info {
		itr->first,
		itr->second.created,
		itr->second.modified,
		itr->second.buffer.size()
	};

	return info;
}

bool isSupportedFileExtension(const std::string& filepath, const std::initializer_list<std::string>& extensions) {

	const auto pathNormalized = Strings::toLowerCase(filepath);
	const auto pathExtension = std::filesystem::path(pathNormalized).extension();

	for (const auto& item : pathExtension) {
		if (item == pathExtension) return true;
	}

	return false;
}

void Interface::loadSnapshot(const std::string& path) {

	Formats::FSQueue readerQueue;
	std::future<void> readerPromise;

	if (isSupportedFileExtension(path, Formats::Tar::supportedExtensions)) {

		readerPromise = std::async(Formats::Tar::importArchive, path, std::ref(readerQueue));

		while (readerQueue.await()) {
			
			const auto next = readerQueue.next();
			const auto pathNormalized = Strings::toLowerCase(next.name);
			
			StoredValue vfsentry {
				next.buffer,
				std::time(nullptr),
				next.modified
			};

			std::lock_guard<std::mutex> lock(this->m_lock);
			this->m_data[pathNormalized] = vfsentry;
		}

	} else {
		throw std::runtime_error("loadSnapshot() error: vfs uses file extension to determine storage file format and it was not recognized");
	}

	if (!readerQueue.done()) {
		readerQueue.close();
	}

	if (readerPromise.valid()) {
		readerPromise.get();
	}
}

void Interface::saveSnapshot(const std::string& path) {

	Formats::FSQueue writerQueue;
	std::future<void> writerPromise;

	if (isSupportedFileExtension(path, Formats::Tar::supportedExtensions)) {

		writerPromise = std::async(Formats::Tar::exportArchive, path, std::ref(writerQueue));

		std::lock_guard<std::mutex> lock(this->m_lock);

		for (const auto& entry : this->m_data) {

			if (!writerQueue.awaitEmpty()) break;

			VirtualFile vfsfile {
				entry.second.buffer,
				entry.second.created,
				entry.second.modified,
				entry.first
			};

			writerQueue.push(std::move(vfsfile));
		}

	} else {
		throw std::runtime_error("saveSnapshot() error: vfs uses file extension to determine storage file format and it was not recognized");
	}

	if (!writerQueue.done()) {
		writerQueue.close();
	}

	if (writerPromise.valid()) {
		writerPromise.get();
	}
}

const VFSInfo& Interface::stats() const noexcept {
	return this->m_info;
}
