#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "../log/log.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto fs_reader = FsDirectoryServe("utils/testassets/static");
	auto fss = FileServer(fs_reader, { .debug = true });

	auto server = Lambda::Server(fss.handler_fn(), { .debug = true });
	server.serve();

	return 0;
}
