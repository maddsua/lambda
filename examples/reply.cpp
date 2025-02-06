//	Reply with client's user agent

#include <cstdio>

#include "../lambda.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {
	const auto uaHeader = req.headers.get("user-agent");
	const auto uaString = uaHeader.size() ? uaHeader : "Unknown";
	wrt.write("Your user-agent is: " + uaString);
};

int main() {
	auto server = Lambda::Server(handler_fn, { .debug = true });
	printf("Listening at: http://localhost:%i/\n", server.options.port);
	server.serve();
	return 0;
}
