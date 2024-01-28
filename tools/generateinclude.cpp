#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <unordered_set>
#include <regex>

#include "../version.hpp"
#include "../core/polyfill/polyfill.hpp"

struct Options {
	std::string inputHeader;
	std::string outputHeader;
};

struct IncludeCtx {
	std::unordered_set<std::string> imported;
};

std::string includeHeader(const std::string& inputHeader, IncludeCtx& ctx);

struct RecolvePathResult {
	std::string path;
	bool embeddable;
};

RecolvePathResult resolveHeaderIncludePath(const std::string input);

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

	IncludeCtx context;

	includeHeader(opts.inputHeader, context);

	return 0;
}

RecolvePathResult resolveHeaderIncludePath(const std::string input) {

	size_t startIdx = std::string::npos;
	bool isStdHeader = false;

	auto pathStart = input.find('\"');
	if (pathStart == std::string::npos)
		throw std::runtime_error("invalid include statement path start");

	for (auto sym : std::initializer_list<char>({ '\"', '<' })) {

	}

}

std::string includeHeader(const std::string& inputHeaderPath, IncludeCtx& ctx) {

	std::ifstream file(inputHeaderPath);

	if (!file.is_open()) {
		throw std::runtime_error("ahhh shit could'nt read the file \"" + inputHeaderPath + "\"\n");
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line))
		lines.push_back(line);

	file.close();

	std::vector<std::string> resultingLines;

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

		auto basePath = std::filesystem::path(inputHeaderPath).parent_path();
		auto resolvedPath = std::filesystem::relative(filePath, basePath.generic_string().size() ? basePath : "./").generic_string();

		if (ctx.imported.contains(resolvedPath)) continue;

		ctx.imported.insert(resolvedPath);
		resultingLines.push_back(includeHeader(resolvedPath, ctx));
	}

	for (const auto& item : resultingLines) {
		puts(item.c_str());
	}

	return {};
}
