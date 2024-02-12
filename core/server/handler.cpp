#include "./internal.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

void Server::connectionHandler(
	Lambda::Network::TCP::Connection&& conn,
	const ServeOptions& config,
	const ServerlessCallback& handlerCallback
) {

	const auto contextID = Crypto::ShortID().toString();
	const auto& conninfo = conn.info();
	auto connctx = TransportContextV1(conn, config.transport);
	std::optional<std::exception> transportError;

	if (config.loglevel.transportEvents) {
		syncout.log({
			"[Transport]",
			conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
			"created",
			contextID
		});
	}

	try {

		while (connctx.awaitNext()) {
			
			const auto next = connctx.nextRequest();
			const auto requestID = Crypto::ShortID().toString();

			HTTP::Response response;
			std::optional<std::string> handlerError;

			try {

				response = handlerCallback(next, {
					contextID,
					requestID,
					conninfo
				});

			} catch(const std::exception& e) {
				handlerError = e.what();
			} catch(...) {
				handlerError = "unhandled exception";
			}

			if (handlerError.has_value()) {

				if (config.loglevel.requests) {
					syncout.error({
						"[Serverless]",
						requestID,
						"crashed:",
						handlerError.value()
					});
				}

				response = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);
			}

			response.headers.set("x-request-id", contextID + '-' + requestID);
			connctx.respond(response);

			if (config.loglevel.requests) {
				syncout.log({
					"[Serverless]",
					(config.loglevel.transportEvents ? contextID + '-' : conninfo.remoteAddr.hostname + ' ') + requestID,
					next.method.toString(),
					next.url.pathname,
					"-->",
					response.status.code()
				});
			}
		}

	} catch(const std::exception& err) {
		transportError = std::move(err);
	} catch(...) {
		transportError = std::runtime_error("Unknown exception");
	}
	
	if (transportError.has_value() && (config.loglevel.transportEvents || config.loglevel.requests)) {

		auto transportDisplayID = contextID;
		if (!config.loglevel.transportEvents) {
			transportDisplayID += '(' + conninfo.remoteAddr.hostname + 
				':' + std::to_string(conninfo.remoteAddr.port) + ')';
		}

		syncout.error({
			"[Transport]",
			transportDisplayID,
			"terminated:",
			transportError.value().what()
		});

	} else if (config.loglevel.transportEvents) {
		syncout.log({
			"[Transport]",
			contextID,
			"closed ok"
		});
	}
}
