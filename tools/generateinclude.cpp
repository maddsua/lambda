#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <filesystem>

#include "../version.hpp"
#include "../core/polyfill/polyfill.hpp"

struct Options {
	std::string inputHeader;
	std::string outputHeader;
};

int main(int argc, char const *argv[]) {

	Options opts;

	for (int i = 0; i < argc; i++) {

		const auto arg = std::string(argv[i]);
		if (!arg.starts_with("--")) continue;

		auto separatorPos = arg.find('=');
		if (separatorPos == std::string::npos) continue;

		auto key = arg.substr(2, separatorPos - 2);
		auto value = arg.substr(separatorPos + 1);

		if (key == "entrypoint") opts.inputHeader = value;
		else if (key == "output") opts.outputHeader = value;
	}

	try {

		if (!opts.inputHeader.size())
			throw std::runtime_error("ABORTED: Provide entrypoint file with --entrypoint=[filelocation]");

		if (!opts.outputHeader.size())
			throw std::runtime_error("ABORTED: Provide output file with --output=[filelocation]");

	} catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}

	std::ifstream file(opts.inputHeader);

	if (!file.is_open()) {
		std::cerr << "ahhh shit could'nt read the file \"" << opts.inputHeader << "\"\n";
		return 1;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
		lines.push_back(line);

	file.close();

	std::vector<std::pair<std::string, std::string>> replacementPairs;

	for (const auto& line : lines) {

		if (!Lambda::Strings::trim(line).starts_with("#include")) continue;
		else if (Lambda::Strings::includes(line, '<')) continue;

		auto pathStart = line.find('\"');
		if (pathStart == std::string::npos)
			throw std::runtime_error("invalid include statement path start");

		auto pathEnd = line.find('\"', pathStart + 1);
		if (pathEnd == std::string::npos)
			throw std::runtime_error("invalid include statement path end");

		auto filePath = line.substr(pathStart + 1, pathEnd - pathStart - 1);

		auto basePath = std::filesystem::path(opts.inputHeader).parent_path();
		std::filesystem::path resolvedPath = std::filesystem::relative(filePath, basePath.generic_string().size() ? basePath : "./");

		replacementPairs.push_back({ line, resolvedPath.generic_string() });
	}

	for (const auto& item : replacementPairs) {
		puts(item.second.c_str());
	}

	return 0;
}
