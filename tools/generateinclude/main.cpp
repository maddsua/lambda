#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <unordered_set>
#include <regex>

#include "../../version.hpp"

using namespace std::regex_constants;

extern char _binary_tools_generateinclude_headerfile_txt_start;
extern char _binary_tools_generateinclude_headerfile_txt_end;

struct Options {
	std::string inputHeader;
	std::string outputHeader;
};

struct IncludeCtx {
	std::unordered_set<std::string> imported;
	std::vector<std::string> globalIncludeList;
};

struct RecolvePathResult {
	std::string path;
	bool embeddable;
};

RecolvePathResult parseIncludePath(const std::string input);
std::string resolvePath(const std::string& a, const std::string& b);
std::string includeHeader(const std::string& inputHeader, IncludeCtx& ctx);

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
	auto mergedIncludes = includeHeader(opts.inputHeader, context);

	std::string globalIncludes;
	for (const auto& item : context.globalIncludeList) {
		globalIncludes.append("\n");
		globalIncludes.append(item);
	}

	auto templateReplacements = std::initializer_list<std::pair<std::string, std::string>>({
		{ "release_time", "2024" },
		{ "global_includes", globalIncludes },
		{ "merged_includes", mergedIncludes },
	});

	std::string result = std::string(
		&_binary_tools_generateinclude_headerfile_txt_start,
		&_binary_tools_generateinclude_headerfile_txt_end - &_binary_tools_generateinclude_headerfile_txt_start
	);

	for (const auto& item : templateReplacements) {
		auto varexpr = std::regex("\\{\\s*\\{\\s*" + item.first + "\\s*\\}\\s*\\}", ECMAScript | icase);
		result = std::regex_replace(result, varexpr, item.second);
	}

	printf("Writing \"%s\"\n", opts.outputHeader.c_str());

	auto sfiHeader = std::fstream(opts.outputHeader, std::ios::out | std::ios::binary);
	if (!sfiHeader.is_open()) throw std::runtime_error("Couldn't open file " + opts.outputHeader + " for write");
	sfiHeader.write(result.data(), result.size());

	return 0;
}

RecolvePathResult parseIncludePath(const std::string input) {

	size_t startIdx = std::string::npos;

	auto includePath = std::regex_replace(std::regex_replace(input, std::regex("^\\s*\\#include\\s*", ECMAScript | icase), ""), std::regex("\\s*$"), "");
	bool isStdHeader = includePath.starts_with("<");

	return {
		isStdHeader ? includePath : includePath.substr(1, includePath.size() - 2),
		!isStdHeader
	};
}

std::string resolvePath(const std::string& base, const std::string& path) {

	auto basePath = std::filesystem::relative(base).remove_filename().generic_string();
	auto resolved = std::filesystem::relative(basePath + path).generic_string();

	printf("--> Including \"%s\"...\n", resolved.c_str());
	
//	printf("%s | %s | %s | %s\n", base.c_str(), basePath.c_str(), path.c_str(), resolved.c_str());
	return resolved;
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

		if (!std::regex_match(line, std::regex("^\\s*\\#include.*$", ECMAScript | icase))) {
			resultingLines.push_back(line);
			continue;
		}

		auto includePath = parseIncludePath(line);
		auto resolvedPath = includePath.embeddable ? resolvePath(inputHeaderPath, includePath.path) : includePath.path;
		
		if (ctx.imported.contains(resolvedPath)) continue;
		ctx.imported.insert(resolvedPath);

		if (!includePath.embeddable) {
			ctx.globalIncludeList.push_back("#include " + includePath.path);
			continue;
		}

		resultingLines.push_back(includeHeader(resolvedPath, ctx));
	}

	std::string result;
	for (const auto& item : resultingLines) {
		result.insert(result.end(), item.begin(), item.end());
		result.push_back('\n');
	}

	//	remove include guards
	result = std::regex_replace(result, std::regex("\\s*\\#ifndef.+LIB.*LAMBDA.+[\\r\\n]+\\s*\\#define.*"), "");
	result = std::regex_replace(result, std::regex("[\\r\\n\\s]+\\#endif[\\r\\n\\s]+$"), "\n");

	//	remove comment blocks
	result = std::regex_replace(result, std::regex("\\/\\*[\\r\\n\t ](\\*(?!\\/)|[^*])*\\*\\/", ECMAScript | icase), "");
	result = std::regex_replace(result, std::regex("[\\r\\n]\\s*\\/{2}.*[\\r\\n]", ECMAScript | icase), "\n");

	//	removed repeating new lines
	auto newlineNormalized = std::regex_replace(result, std::regex("\\r", ECMAScript | icase), "");
	result = std::regex_replace(newlineNormalized, std::regex("\\n{3,}", ECMAScript | icase), "\n\n");

	//	remove empty lines
	result = std::regex_replace(result, std::regex("[\\r\\n][\\t ]+[\\r\\n]", ECMAScript | icase), "");

	return result;
}
