//	an example of using websockets

#include <cstdio>

#include "../lambda.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	if (req.url.path != "/ws") {
		wrt.write_header(Status::NotFound);
		return;
	}

	auto ws = Websocket(req, wrt);

	ws.write("Redy to 'sign' your messages");

	while (ws.is_open()) {
		
		auto next = ws.next();
		if (!next.has_value()) {
			continue;
		}

		auto msg = next.value();

		switch (msg.code) {
	
			case Ws::Opcode::Ping: {

				puts("--> Pong");

				ws.write(Frame {
					.code = Ws::Opcode::Pong,
					.data = msg.data
				});

			} break;

			case Ws::Opcode::Text: {
				auto signed_message = "Signed message: '" + std::string(msg.data.begin(), msg.data.end()) + "'; Signature: lambda";
				ws.write(signed_message);
				puts("--> 'Signed' client message");
			} break;

			case Ws::Opcode::Close: {
				puts("--> See you later aligator!");
				return;
			} break;
		
			default: {
				puts("--> Received an unsupported (by the example app) message; Ignoring");
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
