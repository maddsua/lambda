#include <iostream>

#include "../../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) -> HandlerResponse {

		auto wsctx = context.upgrateToWebsocket();

		wsctx.sendMessage({ "Hi from the server!" });

		while (wsctx.awaitMessage()) {

			auto nextmsg = wsctx.nextMessage();

			syncout.log({ "Message:", nextmsg.text() });

			wsctx.sendMessage({ "Copied that" });
		}

		return {};
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	initparams.loglevel.transportEvents = true;
	auto server = LambdaInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
