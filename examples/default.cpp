#include <iostream>

#include "../lambda.hpp"
#include "../core/server/http.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {
		return HTTPServer::errorResponse(200, "this is a test message");
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
