#include <cstdio>
#include <thread>

#include "../server/server.hpp"
#include "../log/log.hpp"
#include "./fs.hpp"

using namespace Lambda;

int main() {

	auto reader = std::make_shared<DirReader>(DirReader("utils/testassets/static"));
	auto handler = std::make_shared<StaticServer>(StaticServer(reader, { .debug = true }));
	auto server = Lambda::Server(handler, { .debug = true });

	server.serve();

	return 0;
}
