#include <cstdio>
#include <thread>

#include "./websocket.hpp"
#include "../server/server.hpp"
#include "../http/http_private.hpp"

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

				printf("--> Responding to ping\n");

				ws.write(Frame {
					.code = Ws::Opcode::Pong,
					.data = msg.data
				});

			} break;

			case Ws::Opcode::Text: {
				printf("--> Received message: [%s]\n", std::string(msg.data.begin(), msg.data.end()).c_str());
			} break;

			case Ws::Opcode::Close: {
				printf("--> Received a close signal. Bye-bye!\n");
				return;
			} break;
		
			default: {
				printf("--> Received a frame (%i) length: %i\n", static_cast<int>(msg.code), static_cast<int>(msg.data.size()));
			} break;
		}
	}
}

int main() {

	auto server = Lambda::Server(handler_fn, { .debug = true });
	
	printf("Listening at: http://localhost:%i/\n", server.options.port);

	server.serve();

	return 0;
}
