#include <cstdio>
#include <thread>

#include "./server.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	printf("--> [%s] %i %s (%s)\n", req.url.host.c_str(), static_cast<std::underlying_type_t<Method>>(req.method), req.url.to_string().c_str(), req.body.text().c_str());

	if (req.url.path == "/") {
		const std::string message = "yo mr white";
		wrt.header().set("content-type", "text/plain");
		wrt.header().set("content-length", std::to_string(message.size()));
		wrt.write(message);
		return;
	}

	wrt.write_header(Lambda::Status::NotFound);
}

int main() {

	auto server = Lambda::Server(handler_fn, { .debug = true });
	
	printf("Listening at: http://localhost:%i/\n", server.options.port);

	server.Serve();

	return 0;
}
