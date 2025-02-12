//	An example of a simple static server that serves a NextJS project

//	IMPORTANT: You'd need to clone the 'https://github.com/maddsua/lambda-next-app'
// to the same parent directory as this project, and run 'npm run build' there

#include <cstdio>
#include <thread>

#include "../lambda.hpp"

using namespace Lambda;

const std::string www_dist = "../lambda-next-app/out";

int main() {

	auto reader = std::make_shared<DirReader>(DirReader(www_dist));
	auto handler = std::make_shared<StaticServer>(StaticServer(reader, { .debug = true }));

	auto server = Lambda::Server(handler, { .debug = true });
	server.serve();

	return 0;
}
