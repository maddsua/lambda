/**
 * This is a web server example app using Lambda
 * It just serves static content. Probably a Vercel killer of some sort lol
*/

#include "../lambda.hpp"
#include <iostream>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Encoding;
using namespace Lambda::Storage;

struct ServerPass {
	VFS* vfs;
};

HTTP::Response callbackServerless(Request& request, Context& context) {

	std::cout << "Request to \"" << request.url.pathname << "\" from " << context.clientIP << std::endl;

	if (context.passtrough == nullptr) {
		return Response(500, {}, "HTTP/500 Error: VFS unavailable");
	}

	const auto vfs = ((ServerPass*)context.passtrough)->vfs;

	//	transform request url a bit
	auto filepath = request.url.pathname;
	if (filepath.ends_with("/")) filepath.replace(filepath.size() - 1, 1, "/index.html");
	if (filepath.starts_with("/")) filepath.erase(0, 1);

	//	find file in vfs
	auto file = vfs->read(filepath);
	if (!file.size()) return Response(404, {}, "HTTP/404 Error: File not found");

	auto response = Response();

	response.body = file;

	auto pathExtIdx = filepath.find_last_of('.');
	if (pathExtIdx != std::string::npos)
		response.headers.append("content-type", getExtMimetype(filepath.substr(pathExtIdx)));

	return response;
};

int main() {

	const int port = 8080;

	auto server = Lambda::Server(port);

	server.flags.compressionUseBrotli = true;
	server.flags.compressionUseGzip = true;

	ServerPass passthough;
	passthough.vfs = new VFS();

	auto vfsload = passthough.vfs->loadSnapshot("examples/content/website.dist.tar.gz");
	if (vfsload.isError()) std::cout << "Failed to load VFS: " << vfsload.what() << std::endl;
		else server.enablePasstrough(&passthough);

	std::cout << "Server started at http://localhost:" + std::to_string(port) << std::endl;

	server.setServerlessCallback(&callbackServerless);

	while (server.isAlive()) {
		
		if (server.hasNewLogs()) {
			puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return 0;
}
