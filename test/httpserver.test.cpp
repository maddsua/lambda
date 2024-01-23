#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

HTTP::Response httpHandler(const Request& req, const Context& context) {

	context.console.log({"Serving rq for:", req.url.pathname});

	if (req.body.size()) context.console.log({"Request payload", req.body.text()});

	return HTTP::Response("status report: live");
}

int main(int argc, char const *argv[]) {

	ServerConfig initparams;
	auto server = Server(httpHandler, initparams);
	server.awaitFinished();

	return 0;
}
