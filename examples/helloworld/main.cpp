
/**
 * Example
 * 
 * Simple hello world example
 * 
 * If not this comment, it would be even shorter than express'es one
*/

#include "../../lambda.hpp"
using namespace Lambda;

auto requestHandler = [](const Request& req, const Context& context) {
	return Response("<h1>Hello World!<h1>");
};

int main(int argc, char const *argv[]) {
	ServerConfig opts;
	opts.transport.reuseConnections = false;
	auto server = LambdaInstance(requestHandler, opts);
	server.awaitFinished();
	return 0;
}
