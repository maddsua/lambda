#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto fs_reader = FsDirectoryServe("utils/testassets/static");
	auto fss = FileServer(fs_reader);
	fss.debug = true;

	auto server = Lambda::Server(fss.handler_fn(), { .debug = true });

	printf("Serves files at: http://localhost:%i/\n", server.options.port);

	server.serve();

	return 0;
}
