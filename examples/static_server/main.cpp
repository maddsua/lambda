#include <iostream>

#include "../../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	StaticServer sserver("./dist");

	auto handler = [&](const Request& req, const Context& context) {
		return sserver.serve(req);
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = LambdaInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
