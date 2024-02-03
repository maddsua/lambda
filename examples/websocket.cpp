#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](Lambda::IncomingConnection& conn) {

		auto wsctx = conn.upgrateToWebsocket();

		while (wsctx.awaitMessage()) {
			auto nextmsg = wsctx.nextMessage();
			printf("Message: %s\n", nextmsg.text().c_str());
			wsctx.sendMessage({ "Copied that" });
			wsctx.close(Websocket::CloseReason::GoingAway);
		}
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
