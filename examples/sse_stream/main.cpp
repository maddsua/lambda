#include <iostream>

#include "../../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [](const Request& req, const Context& context) -> HandlerResponse {

		if (req.url.pathname != "/") {
			return HTTP::Response(404, "not found ");
		}

		auto writer = context.startEventStream();

		size_t numberOfMessages = 0;

		while (writer.connected()) {

			writer.push({
				"test message " + std::to_string(numberOfMessages)
			});

			numberOfMessages++;
			if (numberOfMessages > 5) break;

			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		writer.close();
		return {};
	};

	ServerConfig initparams;
	initparams.loglevel.transportEvents = true;
	auto server = LambdaInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
