#include "../include/maddsua/fs.hpp"

bool maddsua::writeBinary(const std::string path, const std::string* data) {

	if (path.find('/') != std::string::npos || path.find('\\') != std::string::npos) {
		auto dirpath = std::regex_replace(path, std::regex("\\+"), "/");
		dirpath = dirpath.substr(0, dirpath.find_last_of('/'));

		DIR* dir = opendir(dirpath.c_str());
		if (!dir) mkdir(dirpath.c_str());
		else closedir(dir);
	}

	FILE* binfile = fopen64(path.c_str(), "wb");
	if (!binfile) return false;

	if (fwrite(&data->at(0), 1, data->size(), binfile) != data->size()) {
		fclose(binfile);
		return false;
	};
	fclose(binfile);

	return true;
}

bool maddsua::readBinary(std::string path, std::string* dest) {
	FILE* binfile = fopen64(path.c_str(), "rb");
	if(!binfile) return false;

	while(!feof(binfile)) {
		uint8_t fileChunk[MADDSUA_FS_READ_CHUNK];
		size_t bytesRead = fread(fileChunk, 1, MADDSUA_FS_READ_CHUNK, binfile);
		if (ferror(binfile)) return false;
		dest->insert(dest->end(), fileChunk, fileChunk + bytesRead);
	}
	fclose(binfile);

	return true;
}