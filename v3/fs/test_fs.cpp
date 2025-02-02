#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto fs_reader = FsDirectoryServe("utils/testassets/static");
	auto fss = FileServer(fs_reader);
	fss.debug = true;

	auto server = Lambda::Server(fss.handler(), { .debug = true });

	printf("Service files at: http://localhost:%i/\n", server.options.port);

	server.Serve();

	return 0;
}
