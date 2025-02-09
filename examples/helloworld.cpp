//	Simple hello world example

#include "../lambda.hpp"
using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {
	wrt.header().set("content-type", "text/html");
	wrt.write("<h1>Hello World!<h1>");
};

int main() {
	auto server = Lambda::Server(handler_fn, { .debug = true });
	server.serve();
	return 0;
}
