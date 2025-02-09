#include <cstdio>
#include <thread>

#include "./websocket.hpp"
#include "../server/server.hpp"
#include "../http/http_utils.hpp"
#include "../log/log.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	if (req.url.path != "/ws") {
		wrt.write_header(Status::NotFound);
		return;
	}

	auto ws = Websocket(req, wrt);

	ws.write("Hello there you fine wanderer");

	while (ws.is_open()) {
		
		auto next = ws.next();
		if (!next.has_value()) {
			continue;
		}

		auto msg = next.value();

		switch (msg.code) {
	
			case Ws::Opcode::Ping: {

				Log::log("--> Responding to ping");

				ws.write(Frame {
					.code = Ws::Opcode::Pong,
					.data = msg.data
				});

			} break;

			case Ws::Opcode::Text: {
				Log::log("--> Received message: [{}]", {
					std::string(msg.data.begin(), msg.data.end())
				});
			} break;

			case Ws::Opcode::Close: {
				Log::log("--> Received a close signal. Bye-bye!");
				return;
			} break;
		
			default: {
				Log::log("--> Received a frame ({}) length: {}", {
					static_cast<int>(msg.code),
					msg.data.size()
				});
			} break;
		}
	}
}

int main() {

	auto server = Lambda::Server(handler_fn, { .debug = true });
	server.serve();

	return 0;
}
