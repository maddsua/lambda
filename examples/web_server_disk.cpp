/**
 * This is a web server example app using Lambda, but this version does not use VFS, but loads files from hard drive directly
*/

#include "../lambda.hpp"
#include <fstream>
#include <string.h>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Encoding;
using namespace Lambda::Storage;

std::vector<uint8_t> loadFile(const std::string& path);
HTTP::Response callbackServerless(Request& request, Context& context);

struct Passtr {
	std::string rootdir;
};

int main(int argc, char** argv) {

	const int port = 8080;

	auto server = Lambda::Server(port);

	server.flags.compressionUseBrotli = true;
	server.flags.compressionUseGzip = true;

	Passtr pass;

	for (size_t i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-d") && i + 1 < argc) {
			pass.rootdir = argv[i + 1];
			break;
		}
	}

	if (!pass.rootdir.size()) {
		puts("Specify dist directory with: '-d dist_dir_name'");
		pass.rootdir = "dist";
	}

	server.enablePasstrough(&pass);

	puts(("Server started at http://localhost:" + std::to_string(port)).c_str());

	server.setServerlessCallback(&callbackServerless);

	while (server.isAlive()) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return 0;
}

std::vector<uint8_t> loadFile(const std::string& path) {
	std::vector<uint8_t> content;
	auto localfile = std::ifstream(path);
	if (!localfile.is_open()) return {};
	content.assign(std::istreambuf_iterator<char>(localfile), std::istreambuf_iterator<char>());
	return content;
}

HTTP::Response callbackServerless(Request& request, Context& context) {

	puts(("Request to \"" + request.url.pathname + "\" from " + context.clientIP).c_str());

	//	transform request url a bit
	auto filepath = request.url.pathname;
	if (filepath.ends_with("/")) filepath.replace(filepath.size() - 1, 1, "/index.html");
	if (filepath.starts_with("/")) filepath.erase(0, 1);

	//	try to load file from fs
	auto rootDir = ((Passtr*)context.passtrough)->rootdir;
	auto file = loadFile(rootDir + "/" + filepath);
	if (!file.size()) return serviceResponse(404, "Resource \"" + request.url.pathname + "\" does not exist<br>Are you sure that you have a '" + rootDir + "' directory next to the executable?");
	
	auto response = Response();
	response.body = file;

	auto pathExtIdx = filepath.find_last_of('.');
	if (pathExtIdx != std::string::npos)
		response.headers.append("content-type", getExtMimetype(filepath.substr(pathExtIdx)));

	return response;
};
