#include "./server_impl.hpp"
#include "../crypto/crypto.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Server;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

void Server::connectionHandler(
	Lambda::Network::TCP::Connection&& conn,
	const ServeOptions& config,
	const RequestCallback& handlerCallback
) noexcept {

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
					'[' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ']',
					"[Serverless]",
					requestID,
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

			if (transport.ok()) {
				try { transport.respond(errorResponse); } 
					catch(...) {}
			}
		}

		transportError = std::move(err);

	} catch(const std::exception& err) {
		transportError = std::move(err);
	} catch(...) {
		transportError = std::runtime_error("Unknown exception");
	}

	if (transportError.has_value() && (config.loglevel.transportEvents || config.loglevel.requests)) {
		syncout.error({
			"[Transport]",
			contextID,
			'(' + conninfo.remoteAddr.hostname + ')',
			"terminated:",
			transportError.value().what()
		});

	} else if (config.loglevel.transportEvents) {
		syncout.log({
			"[Transport]",
			contextID,
			'(' + conninfo.remoteAddr.hostname + ')',
			"disconnected"
		});
	}
}
