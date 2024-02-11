#include <iostream>

#include "../../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](Lambda::IncomingConnection& conn) {

		auto next = conn.nextRequest();
		if (!next.has_value()) return;

		auto writer = conn.startEventStream();

		while (writer.connected()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			writer.push({
				"test message"
			});
		}
	};

	ServerConfig initparams;
	initparams.loglevel.transportEvents = true;
	auto server = LambdaInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
