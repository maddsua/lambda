//	An example of a simple static server that serves a NextJS project

//	IMPORTANT: You'd need to clone the 'https://github.com/maddsua/lambda-next-app'
// to the same parent directory as this project, and run 'npm run build' there

#include <cstdio>
#include <thread>

#include "../lambda.hpp"

using namespace Lambda;

const std::string www_dist = "../lambda-next-app/out";

int main() {

	auto fss = StaticServer(std::make_shared<DirReader>(DirReader(www_dist)), { .debug = true });

	auto server = Lambda::Server(fss.handler_fn(), { .debug = true });
	server.serve();

	return 0;
}
