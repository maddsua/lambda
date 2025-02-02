#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto fss = FileServer(new FsDirectoryServe("utils/testassets/static"));
	fss.debug = true;

	auto server = Lambda::Server(fss.handler(), { .debug = true });
	
	printf("Service files at: http://localhost:%i/\n", server.options.port);

	server.Serve();

	return 0;
}
