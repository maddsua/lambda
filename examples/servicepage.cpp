#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {

		auto templateSource = HTML::Templates::servicePage();

		auto pagehtml = HTML::renderTemplate(templateSource, {
			{ "svcpage_statuscode", "000" },
			{ "svcpage_statustext", "We're live!" },
			{ "svcpage_message_text", "Congrats, you have compiled it xD" }
		});

		return HTTP::Response(pagehtml);
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
