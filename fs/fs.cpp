#include "./fs.hpp"
#include <fstream>
#include <iterator>
#include <dir.h>
#include <dirent.h>

using namespace Lambda;
using namespace Lambda::Fs;

void Fs::normalizePath(std::string& path) {

	auto tempidx = 0;
	
	//	replace backslahes
	while (tempidx != std::string::npos) {
		tempidx = path.find("\\\\");
		path.replace(tempidx, tempidx + 1, "/");
	}

	//	reset indes and run forward slash de-duplication
	tempidx = 0;
	while (tempidx != std::string::npos) {
		tempidx = path.find("//");
		path.replace(tempidx, tempidx + 1, "/");
	}

	//	remove the "./" pattern
	tempidx = 0;
	while (tempidx != std::string::npos) {
		tempidx = path.find("./");
		path.erase(tempidx, tempidx + 1);
	}

	//	remove the "/./" pattern
	tempidx = 0;
	while (tempidx != std::string::npos) {
		tempidx = path.find("/./");
		path.erase(tempidx, tempidx + 2);
	}

}

Error Fs::readFileSync(std::string path, std::vector<uint8_t>& content) {

	normalizePath(path);

	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) return { "Could not open the file for reading" };

	content.clear();
	content.assign(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

	file.close();
	return {};
}

Error Fs::readTextFileSync(std::string path, std::string& content) {

	normalizePath(path);

	std::ifstream file(path);
	if (!file.is_open()) return { "Could not open the text file for reading" };

	content.clear();
	content.assign(std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

	file.close();
	return {};
}

void createDirectoryPath(std::string path) {

	if (!path.size()) return;
	
	auto createIfDontexist = [](const char* pthcstr) {
		auto dir = opendir(pthcstr);
		if (dir) {
			closedir(dir);
			return true;
		}
		if (mkdir(pthcstr)) return false;
		return true;
	};

	auto hierrarchy = path.find_first_of('/');
	while (hierrarchy != std::string::npos) {
		if (!createIfDontexist(path.substr(0, hierrarchy).c_str())) return;
		hierrarchy = path.find_first_of('/', hierrarchy + 1);
	}
}

Lambda::Error Fs::writeFileSync(std::string path, std::vector<uint8_t>& content) {

	normalizePath(path);
	createDirectoryPath(path);

	std::ofstream file(path);
	if (!file.is_open()) return { "Could not open file for write" };

	file.write((char*)content.data(), content.size());
	file.close();
	return {};
}

Lambda::Error Fs::writeTextFileSync(std::string path, std::string& content) {

	normalizePath(path);
	createDirectoryPath(path);

	std::ofstream file(path);
	if (!file.is_open()) return { "Could not open file for write" };

	file.write(content.data(), content.size());
	file.close();
	return {};
}
