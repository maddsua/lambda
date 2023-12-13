#include <fstream>
#include <stdexcept>
#include <iterator>
#include <filesystem>

#include "../fs.hpp"

using namespace Lambda;

bool FS::existsSync(const std::string& filepath) {
	return std::filesystem::exists(filepath);
}

std::vector<uint8_t> FS::readFileSync(const std::string& filepath) {

	auto filestream = std::fstream(filepath, std::ios::in | std::ios::binary);
	if (!filestream.is_open()) throw std::runtime_error("File \"" + filepath + "\" could not be opened");

	filestream.seekg(0, std::ios::end);
	auto fileSize = filestream.tellg();
	filestream.seekg(0, std::ios::beg);

	std::vector<uint8_t> rawcontent;
	rawcontent.reserve(fileSize);

	rawcontent.insert(rawcontent.begin(), std::istream_iterator<uint8_t>(filestream), std::istream_iterator<uint8_t>());
	filestream.close();

	return rawcontent;
}

std::string FS::readTextFileSync(const std::string& filepath) {

	auto filestream = std::fstream(filepath, std::ios::in);
	if (!filestream.is_open()) throw std::runtime_error("File \"" + filepath + "\" could not be opened");

	std::string textcontent;
	std::string temp;

	while (std::getline(filestream, temp)) {
		textcontent.append(temp + '\n');
	}
	filestream.close();

	return textcontent;
}

void FS::writeFileSync(const std::string& filepath, const std::vector<uint8_t>& data) {

	auto filestream = std::fstream(filepath, std::ios::out | std::ios::binary);
	if (!filestream.is_open()) throw std::runtime_error("File \"" + filepath + "\" could not be opened");

	filestream.write((const char*)data.data(), data.size());
	filestream.close();
}

void FS::writeTextFileSync(const std::string& filepath, const std::string& data) {

	auto filestream = std::fstream(filepath, std::ios::out);
	if (!filestream.is_open()) throw std::runtime_error("File \"" + filepath + "\" could not be opened");

	filestream.write(data.data(), data.size());
	filestream.close();
}
