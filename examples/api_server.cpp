#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto storage = Lambda::Storage::LocalStorage();

	auto handler = [&](const Request& req, const Context& context) {

		
		JSON::Map testMap = {
			{"date", Date().toUTCString()},
			{"useragent", req.headers.get("user-agent")},
		};

		auto response = HTTP::Response(stringify(Property(testMap)));

		return response;
	};

	HttpServerConfig initparams;
	initparams.loglevel.logRequests = true;
	auto server = HttpServer(handler, initparams);
	server.awaitFinished();

	return 0;
}
