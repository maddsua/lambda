#include "../include/maddsua/fs.hpp"

bool maddsuaFS::writeBinary(std::string path, std::vector<uint8_t>* data) {

	if (path.find('/') || path.find('\\')) {
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

bool maddsuaFS::readBinary(std::string path, std::vector<uint8_t>* dest) {
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