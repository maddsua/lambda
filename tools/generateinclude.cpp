#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "../version.hpp"

struct Options {
	std::string inputHeader;
	std::string outputHeader;
};

std::string loadFile(const std::string& path) {
	std::string content;
	auto localfile = std::ifstream(path);
	if (!localfile.is_open()) throw std::runtime_error("Couldn't open file " + path + " for read");
	content.assign(std::istreambuf_iterator<char>(localfile), std::istreambuf_iterator<char>());
	return content;
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

		if (key == "template") opts.inputHeader = value;
		else if (key == "output") opts.outputHeader = value;
	}

	try {

		if (!opts.inputHeader.size())
			throw std::runtime_error("ABORTED: Provide template file with --template=[filelocation]");

		if (!opts.outputHeader.size())
			throw std::runtime_error("ABORTED: Provide output file with --output=[filelocation]");

	} catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}


	return 0;
}
