#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {
		throw std::runtime_error("test error");
		return HTTP::Response();
	};

	ServerConfig initparams;
	initparams.errorResponseType = ErrorResponseType::JSON;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
