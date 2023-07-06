#include "./vfs.hpp"
#include <time.h>
#include <fstream>

using namespace Lambda;
using namespace Lambda::Storage;

Lambda::Error VFS::read(const std::string& path, std::vector <uint8_t>& content) {
	std::lock_guard <std::mutex> lock(threadLock);
	if (!this->vfs.contains(path)) return { "Virtual file \"" + path + "\" not found" };
	content = this->vfs.at(path).content;
	return {};
}

Lambda::Error VFS::write(const std::string& path, const std::vector <uint8_t>& content) {

	VirtualFile vfile;

	std::lock_guard <std::mutex> lock(threadLock);

	if (!this->vfs.contains(path)) {
		vfile.created = time(nullptr);
		vfile.modified = time(nullptr);
	} else vfile = this->vfs.at(path);

	this->vfs[path] = vfile;

	return {};
}

Lambda::Error VFS::remove(const std::string& path) {

	std::lock_guard <std::mutex> lock(threadLock);

	if (!this->vfs.contains(path)) return { "Virtual file \"" + path + "\" already removed or just not there" };

	this->vfs.erase(path);

	return {};
}

Lambda::Error VFS::rename(const std::string& path, const std::string& newPath) {

	std::lock_guard <std::mutex> lock(threadLock);

	if (!this->vfs.contains(path)) return { "Virtual file \"" + path + "\" not found" };
	if (this->vfs.contains(newPath)) return { "Virtual file \"" + newPath + "\" already exists" };

	this->vfs[newPath] = this->vfs.at(path);
	this->vfs.erase(path);

	return {};
}

std::vector<VirtualFileInfo> VFS::listAll() {

	std::vector<VirtualFileInfo> listresult;

	std::lock_guard <std::mutex> lock(threadLock);

	for (const auto& file : this->vfs) {

		VirtualFileInfo temp;
		temp.name = file.first;
		temp.modified = file.second.modified;
		temp.created = file.second.created;
		temp.size = file.second.content.size();
		
		listresult.push_back(std::move(temp));
	}

	return listresult;
}

VirtualFileInfo VFS::fileInfo(const std::string& path) {

	VirtualFileInfo info;
	info.name = path;

	if (!vfs.contains(path)) {
		info.found = false;
		return info;
	}

	const auto& file = this->vfs.at(path);
	info.modified = file.modified;
	info.created = file.created;
	info.size = file.content.size();

	return info;
}

Lambda::Error VFS::loadSnapshot(const std::string& path) {

	auto filestream = std::ifstream(path, std::ios::binary);
	if (!filestream.is_open()) return { "Could not open the file \"" + path + "\"" };

	try {
		importTar(filestream);
	} catch(const std::exception& e) {
		return { std::string("Tar import failed: ") + e.what() };
	} catch(...) {
		return { "Unhandled tar import error" };
	}

	return {};
}

Lambda::Error VFS::saveSnapshot(const std::string& path) {

	auto filestream = std::ofstream(path, std::ios::binary);
	if (!filestream.is_open()) return { "Could not open the file \"" + path + "\"" };

	try {

		auto compression = TARCOMPRESS_NONE;

		if (path.ends_with(".gz")) compression = TARCOMPRESS_GZIP;

		exportTar(filestream, compression);

	} catch(const std::exception& e) {
		return { std::string("Tar export failed: ") + e.what() };
	} catch(...) {
		return { "Unhandled tar export error" };
	}

	return {};
}
