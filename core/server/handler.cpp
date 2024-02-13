#include "./server_impl.hpp"
#include "../http/transport.hpp"
#include "../http/transport_impl.hpp"
#include "../crypto/crypto.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Server;
using namespace Lambda::Server::Connections;
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

	const auto handleTransportError = [&](const std::exception& error) -> void {

		if (!(config.loglevel.transportEvents || config.loglevel.requests)) {
			return;
		}

		syncout.error({
			"[Transport]",
			contextID,
			'(' + conninfo.remoteAddr.hostname + ')',
			"terminated:",
			error.what()
		});
	};

	const auto handleProtocolError = [&](const ProtocolError& error) -> void {

		if (!error.respondStatus.has_value() || !transport.isConnected()) {
			return;
		}

		try {

			const auto errorResponse = Pages::renderErrorPage(
				error.respondStatus.value(),
				error.message(),
				config.errorResponseType
			);

			transport.respond(errorResponse);
			transport.close();

		} catch(...) {

			if (!config.loglevel.transportEvents) {
				return;
			}

			syncout.error({
				"[Transport]",
				"Failed to respond to protocol error in",
				contextID,
				"(connection dropped)"
			});
		}
	};

	if (config.loglevel.transportEvents) {
		syncout.log({
			"[Transport]",
			conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
			"created",
			contextID
		});
	}

	try {

		while (transport.isConnected() && handlerMode == HandlerMode::HTTP && transport.awaitNext()) {

			const auto next = transport.nextRequest();
			const auto requestID = Crypto::ShortID().toString();
			std::optional<std::string> requestUpgraded;

			const auto logConnectionUpgrade = [&](const std::string& protocol) {

				requestUpgraded = protocol;

				if (!config.loglevel.requests) return;

				syncout.log({
					'[' + requestID + ']',
					'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
					next.method.toString(),
					next.url.pathname,
					"-->",
					protocol
				});
			};

			const std::function<SSE::Writer()> upgradeCallbackSSE = [&]() {
				handlerMode = HandlerMode::SSE;
				logConnectionUpgrade("SSE stream");
				return SSE::Writer(transport, next);
			};

			const std::function<WebsocketContext()> upgradeCallbackWS = [&]() {
				handlerMode = HandlerMode::WS;
				logConnectionUpgrade("Websocket");
				return WebsocketContext(transport, next);
			};

			const RequestContext requestCTX = {
				contextID,
				requestID,
				conninfo,
				upgradeCallbackSSE,
				upgradeCallbackWS,
			};

			typedef std::optional<HTTP::Response> FunctionResponse;
			FunctionResponse functionResponse;

			const auto handleFunctionError = [&](const std::exception& error) -> FunctionResponse {

				if (config.loglevel.requests) {
					syncout.error({
						'[' + requestID + ']',
						'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
						"Handler crashed:",
						error.what()
					});
				}

				if (handlerMode != HandlerMode::HTTP) {
					return std::nullopt;
				}

				return Pages::renderErrorPage(500, error.what(), config.errorResponseType);
			};

			const auto handleUpgradeError = [&](const UpgradeError& error) -> HTTP::Response {

				handlerMode = HandlerMode::HTTP;

				if (config.loglevel.requests) {
					syncout.error({
						'[' + requestID + ']',
						'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
						"Protocol switch aborted:",
						error.what()
					});
				}

				auto response = Pages::renderErrorPage(error.status(), error.what(), config.errorResponseType);
				response.headers.set("connection", "close");

				return response;
			};

			try {
				functionResponse = handlerCallback(next, requestCTX).response;
			} catch(const UpgradeError& err) {
				handlerMode = HandlerMode::HTTP;
				functionResponse = handleUpgradeError(err);
			} catch(const std::exception& err) {
				functionResponse = handleFunctionError(err);
			} catch(...) {
				functionResponse = handleFunctionError(std::runtime_error("Unexpected handler exception"));
			}

			if (handlerMode == HandlerMode::HTTP) {

				if (!functionResponse.has_value()) {
					functionResponse = Pages::renderErrorPage(500, "Handler failed to respond", config.errorResponseType);
				}

				auto& response = functionResponse.value();
				response.headers.set("x-request-id", contextID + '-' + requestID);
				transport.respond(response);

				if (config.loglevel.requests) {
					syncout.log({
						'[' + requestID + ']',
						'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
						next.method.toString(),
						next.url.pathname,
						"-->",
						response.status.code()
					});
				}

			} else if (config.loglevel.requests && requestUpgraded.has_value()) {
				syncout.log({
					'[' + requestID + ']',
					'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
					"Closed",
					requestUpgraded.value(),
				});
			}
		}

	} catch(const ProtocolError& err) {
		handleProtocolError(err);
		return handleTransportError(err);
	} catch(const std::exception& err) {
		return handleTransportError(err);
	} catch(...) {
		return handleTransportError(std::runtime_error("Unknown exception"));
	}

	if (config.loglevel.transportEvents) {
		syncout.log({
			"[Transport]",
			contextID,
			'(' + conninfo.remoteAddr.hostname + ')',
			"disconnected"
		});
	}
}

HandlerResponse::HandlerResponse(const HTTP::Response& init)  {
	this->response = init;
}

HandlerResponse::HandlerResponse(const std::string& init)  {
	this->response = HTTP::Response(init);
}

HandlerResponse::HandlerResponse(const char* init) {
	this->response = HTTP::Response(init);
}

HandlerResponse::HandlerResponse(const std::vector<uint8_t>& init) {
	this->response = HTTP::Response(200, {
		{ "content-type", "application/octet-stream" }
	}, init);
}
