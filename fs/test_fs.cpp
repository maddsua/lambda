#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "../log/log.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto fss = StaticServer(std::make_shared<DirReader>(DirReader("utils/testassets/static")), { .debug = true });

	auto server = Lambda::Server(fss.handler_fn(), { .debug = true });
	server.serve();

	return 0;
}
