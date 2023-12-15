#include <iostream>
#include <fstream>
#include <stdexcept>

struct Options {
	std::string templateFile;
	std::string outputFile;
	std::string infoFile;
};

std::string loadFile(const std::string& path) {
	std::string content;
	auto localfile = std::ifstream(path);
	if (!localfile.is_open()) throw std::runtime_error("Couldn't open file " + path);
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

		if (key == "template") opts.templateFile = value;
		else if (key == "output") opts.outputFile = value;
		else if (key == "info") opts.infoFile = value;
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
	if (!opts.infoFile.size()) {
		std::cerr << "ABORTED: Provide info file with --info=[filelocation]\n";
		isInitError++;
	}

	if (isInitError) return 1;

	auto templateContent = loadFile(opts.templateFile);
	auto infoContent = loadFile(opts.infoFile);



	return 0;
}
