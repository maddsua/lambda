/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include <regex>
#include <stdio.h>
#include <dir.h>
#include <dirent.h>

#include "../include/lambda/fs.hpp"

bool lambda::fs::createTree(std::string& tree) {

	tree = std::regex_replace(tree, std::regex("[\\\\\\/]+"), "\\");

	if (!tree.size()) return false;

	auto createIfDontexist = [](std::string path) {
		auto dir = opendir(path.c_str());
		if (dir) {
			closedir(dir);
			return true;
		}
		if (mkdir(path.c_str())) return false;
		return true;
	};

	auto hierrarchy = tree.find_first_of('\\');
	while (hierrarchy != std::string::npos) {
		if (!createIfDontexist(tree.substr(0, hierrarchy))) return false;
		hierrarchy = tree.find_first_of('\\', hierrarchy + 1);
	}

	return true;
}

bool lambda::fs::writeSync(std::string path, const std::string* data) {

	FILE* binfile = fopen64(path.c_str(), "wb");
	if (!binfile) return false;

	if (fwrite(&data->at(0), 1, data->size(), binfile) != data->size()) {
		fclose(binfile);
		return false;
	};
	
	fclose(binfile);

	return true;
}

bool lambda::fs::readSync(std::string path, std::string* dest) {

	FILE* binfile = fopen64(path.c_str(), "rb");
	if (!binfile) return false;

	while (!feof(binfile)) {
		uint8_t fileChunk[LAMBDA_FS_READ_CHUNK];
		size_t bytesRead = fread(fileChunk, 1, LAMBDA_FS_READ_CHUNK, binfile);
		if (ferror(binfile)) return false;
		dest->insert(dest->end(), fileChunk, fileChunk + bytesRead);
	}

	fclose(binfile);

	return true;
}