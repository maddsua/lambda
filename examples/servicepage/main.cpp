#include <iostream>

#include "../../lambda.hpp"
#include "../../core/server/server_impl.hpp"

using namespace Lambda;
using namespace Lambda::JSON;
using namespace Lambda::HTML;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {

		auto pagehtml = renderTemplate(Templates::servicePage, {
			{ "svcpage_statuscode", "101" },
			{ "svcpage_statustext", "We're live!" },
			{ "svcpage_message_text", "Congrats, you have compiled it xD" }
		});

		return HTTP::Response(pagehtml);
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = LambdaInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
