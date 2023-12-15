#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {
		context.console.log({"Serving rq for:", req.url.pathname});

		if (req.body.size()) context.console.log({"Request payload", req.body.text()});

		return HTTP::Response("status report: live");
	};

	HttpServerConfig initparams;
	auto server = HttpServer(handler, initparams);
	server.awaitFinished();

	return 0;
}
