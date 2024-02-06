
/**
 * Example
 * 
 * Reply with client's user agent
 * 
 * In this example lambda server will reply back with a message containing client's user-agent string
*/

#include <cstdio>
#include "../../lambda.hpp"

using namespace Lambda;

auto requestHandler = [](const Request& req, const Context& context) {
	const auto uaHeader = req.headers.get("user-agent");
	const auto uaString = uaHeader.size() ? uaHeader : "Unknown";
	return Response("Your user-agent is: " + uaString);
};

int main(int argc, char const *argv[]) {

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = LambdaInstance(requestHandler, initparams);

	server.awaitFinished();

	return 0;
}
