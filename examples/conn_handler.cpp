#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](Lambda::IncomingConnection& conn) {

		while (auto next = conn.nextRequest()) {
			if (!next.has_value()) break;
			printf("Request url: %s\n", next.value().url.href().c_str());
			auto response = HTTP::Response("yo hi there");
			conn.respond(response);
		}
	};

	ServerConfig initparams;
	initparams.loglevel.connections = true;
	auto server = ServerInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
