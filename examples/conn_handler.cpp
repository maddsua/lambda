#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](Lambda::IncomingConnection& conn) {

		auto next = conn.nextRequest();
		if (!next.has_value()) return;

		auto response = HTTP::Response("yo hi there");
		conn.respond(response);
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
