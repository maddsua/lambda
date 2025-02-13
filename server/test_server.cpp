#include <cstdio>
#include <thread>

#include "./server.hpp"
#include "../http/http_utils.hpp"
#include "../log/log.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	Log::log("--> [{}] {} {} {{}}", {
		req.url.host,
		HTTP::method_to_string(req.method),
		req.url.to_string(),
		req.body.text()
	});

	if (req.url.user.has_value()) {
		Log::log("User authorized as '{}' ({})", {
			req.url.user.value().user,
			req.url.user.value().password
		});
	}

	if (req.url.path == "/") {
		const std::string message = "yo mr white";
		wrt.header().set("content-type", "text/plain");
		wrt.header().set("content-length", std::to_string(message.size()));
		wrt.write(message);
		return;
	}

	if (req.url.path == "/events") {

		auto sse = SSEWriter(req, wrt);

		for (size_t idx = 0; idx < 4 && sse.is_open(); idx++) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			sse.push({ .event = "update", .data = "idx=" + std::to_string(idx), });
		}

		return;
	}

	wrt.write_header(Lambda::Status::NotFound);
}

int main() {

	auto server = Lambda::Server(handler_fn, { .debug = true });
	server.serve();

	return 0;
}
