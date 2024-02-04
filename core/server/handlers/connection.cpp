#include "../internal.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::Handlers;

void Handlers::connectionHandler(
	Network::TCP::Connection& conn,
	const ServeOptions& config,
	const ConnectionCallback& handlerCallback
) {

	auto connctx = IncomingConnection(conn, config);
	std::optional<std::string> handlerError;

	try {

		handlerCallback(connctx);

	} catch(const std::exception& e) {
		handlerError = e.what();
	} catch(...) {
		handlerError = "unhandled exception";
	}

	if (handlerError.has_value()) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s crashed: %s\n",
			"tcp handler",
			handlerError.value().c_str()
		);

		auto errorResponse = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);
		connctx.respond(errorResponse);
	}
}
