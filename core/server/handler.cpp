#include "./internal.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
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
	auto handlerMode = HandlerMode::HTTP;
	auto transport = TransportContextV1(conn, config.transport);
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

		while (transport.awaitNext()) {
			
			const auto next = transport.nextRequest();
			const auto requestID = Crypto::ShortID().toString();

			const std::function<SSE::Writer()> upgradeCallbackSSE = [&]() {
				handlerMode = HandlerMode::SSE;
				return SSE::Writer(transport, next);
			};

			const std::function<WebsocketContext()> upgradeCallbackWS = [&]() {
				handlerMode = HandlerMode::WS;
				return WebsocketContext(transport, next);
			};

			const RequestContext requestCTX = {
				contextID,
				requestID,
				conninfo,
				upgradeCallbackSSE,
				upgradeCallbackWS,
			};

			HTTP::Response response;
			std::optional<std::string> handlerError;

			try {
				response = handlerCallback(next, requestCTX);
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

			if (handlerMode == HandlerMode::HTTP) {
				response.headers.set("x-request-id", contextID + '-' + requestID);
				transport.respond(response);
			}

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

	} catch(const ProtocolError& err) {

		/*
			Look. It's not very pretty to rethrow an error but it's way better
			than coming up with	some elaborate structures that will provide a way
			to distinguish between different kinds of errors.
			Also most of the library uses exceptions to do error handling anyway
			so making any of that that would be just super inconsistent and confusing.
		*/
		if (err.respondStatus.has_value()) {
			const auto errorResponse = Pages::renderErrorPage(err.respondStatus.value(), err.message(), config.errorResponseType);
			transport.respond(errorResponse);
		}

		transportError = std::move(err);

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
