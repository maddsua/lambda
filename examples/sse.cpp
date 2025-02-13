//	Stream some SSE events

#include <cstdio>

#include "../lambda.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	if (req.url.path != "/sse") {
		wrt.write_header(Status::NotFound);
		return;
	}

	auto sse = SSEWriter(req, wrt);

	for (size_t idx = 0; idx < 5 && sse.is_open(); idx++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		sse.push(SSEevent { .data = "test message " + std::to_string(idx) });
	}
};

int main() {
	auto server = Lambda::Server(handler_fn, { .debug = true });
	server.serve();
	return 0;
}
