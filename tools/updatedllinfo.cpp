#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "../lambda_version.hpp"

struct Options {
	std::string templateFile;
	std::string outputFile;
};

std::string loadFile(const std::string& path) {
	std::string content;
	auto localfile = std::ifstream(path);
	if (!localfile.is_open()) throw std::runtime_error("Couldn't open file " + path + " for read");
	content.assign(std::istreambuf_iterator<char>(localfile), std::istreambuf_iterator<char>());
	return content;
}

void replaceAll(std::string& base, const std::string& sub, const std::string& replacement) {
	size_t index = base.find(sub);
	while (index != std::string::npos) {
		base.replace(index, sub.size(), replacement);
		index += replacement.size();
		index = base.find(sub, index);
	}
}

int main(int argc, char const *argv[]) {

	Options opts;

	for (int i = 0; i < argc; i++) {

		const auto arg = std::string(argv[i]);
		if (!arg.starts_with("--")) continue;

		auto separatorPos = arg.find('=');
		if (separatorPos == std::string::npos) continue;

		auto key = arg.substr(2, separatorPos - 2);
		auto value = arg.substr(separatorPos + 1);

		if (key == "template") opts.templateFile = value;
		else if (key == "output") opts.outputFile = value;
	}

	int isInitError = false;

	if (!opts.templateFile.size()) {
		std::cerr << "ABORTED: Provide template file with --template=[filelocation]\n";
		isInitError++;
	}
	if (!opts.outputFile.size()) {
		std::cerr << "ABORTED: Provide output file with --output=[filelocation]\n";
		isInitError++;
	}

	if (isInitError) return 1;

	auto templateContent = loadFile(opts.templateFile);



	std::vector<std::pair<std::string, std::string>> replaceList = {
		{ "version_dot", LAMBDA_VERSION },
		{ "version_comma", [&](){
			std::string temp = LAMBDA_VERSION;
			std::string result;
			for (auto symbol : temp) {
				if (isalnum(symbol)) result.push_back(symbol);
				else if (symbol == '.') result.push_back(',');
			}
			return result;
		}() },
		{ "released_year", "2023" },
	};

	auto dllinfoContent = templateContent;

	for (const auto& entry : replaceList) {
		replaceAll(dllinfoContent, '$' + entry.first, entry.second);
	}

	auto dllinfoFile = std::fstream(opts.outputFile, std::ios::out | std::ios::binary);
	if (!dllinfoFile.is_open()) throw std::runtime_error("Couldn't open file " + opts.outputFile + " for write");
	dllinfoFile.write(dllinfoContent.data(), dllinfoContent.size());

	return 0;
}
