#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

HTTP::Response httpHandler(const Request& req, const Context& context) {

	if (req.body.size()) printf("Request payload: %s\n", req.body.text().c_str());

	return HTTP::Response("status report: live");
}

int main(int argc, char const *argv[]) {

	HttpServerInit initparams;
	auto server = HttpServer(httpHandler, initparams);
	server.awaitFinished();

	return 0;
}
