#include <cstdio>
#include <thread>

#include "./server.hpp"
#include "../http/http_utils.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	printf("--> [%s] %s %s (%s)\n",
		req.url.host.c_str(),
		HTTP::method_to_string(req.method).c_str(),
		req.url.to_string().c_str(),
		req.body.text().c_str());

	if (req.url.user.has_value()) {
		printf("[Authorized as %s (%s)]\n",
			req.url.user.value().user.c_str(),
			req.url.user.value().password.c_str());
	}

	if (req.url.path == "/") {
		const std::string message = "yo mr white";
		wrt.header().set("content-type", "text/plain");
		wrt.header().set("content-length", std::to_string(message.size()));
		wrt.write(message);
		return;
	}

	if (req.url.path == "/events") {

		auto sse = SSEWriter(wrt);

		for (size_t idx = 0; idx < 4; idx++) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			sse.write({ .event = "update", .data = "idx=" + std::to_string(idx), });
		}

		return;
	}

	wrt.write_header(Lambda::Status::NotFound);
}

int main() {

	auto server = Lambda::Server(handler_fn, { .debug = true });
	
	printf("Listening at: http://localhost:%i/\n", server.options.port);

	server.serve();

	return 0;
}
