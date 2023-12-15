#include <iostream>
#include <fstream>

struct Options {
	std::string templateFile;
	std::string outputFile;
	std::string infoFile;
};

int main(int argc, char const *argv[]) {

	Options opts;

	for (size_t i = 0; i < argc; i++) {
		const auto arg = std::string(argv[i]);

		auto separatorPos = arg.find('=');
		if (separatorPos == std::string::npos) continue;

		auto key = arg.substr(0, separatorPos);
		auto value = arg.substr(separatorPos + 1);

		if (key == "template") opts.templateFile = value;
		else if (key == "output") opts.outputFile = value;
		else if (key == "info") opts.infoFile = value;
	}

	int isInitError = false;

	if (!opts.infoFile.size()) {
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

	return 0;
}
