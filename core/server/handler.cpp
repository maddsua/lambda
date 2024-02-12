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

	const auto handleTransportError = [&](const std::exception& error) -> void {

		if (!(config.loglevel.transportEvents || config.loglevel.requests)) return;

		syncout.error({
			"[Transport]",
			contextID,
			'(' + conninfo.remoteAddr.hostname + ')',
			"terminated:",
			error.what()
		});
	};

	const auto handleProtocolError = [&](const ProtocolError& error) -> void {

		if (!error.respondStatus.has_value() || !transport.ok()) return;

		try {

			const auto errorResponse = Pages::renderErrorPage(
				error.respondStatus.value(),
				error.message(),
				config.errorResponseType
			);

			transport.respond(errorResponse);

		} catch(...) {

			if (!config.loglevel.transportEvents) return;

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

			typedef std::optional<HTTP::Response> FunctionResponse;
			FunctionResponse functionResponse;

			const auto handleFunctionError = [&](const std::exception& message) -> FunctionResponse {

				if (config.loglevel.requests) {
					syncout.error({
						'[' + requestID + ']',
						'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
						"crashed:",
						message.what()
					});
				}

				if (handlerMode != HandlerMode::HTTP) {
					return std::nullopt;
				}

				return Pages::renderErrorPage(500, message.what(), config.errorResponseType);
			};

			try {
				functionResponse = handlerCallback(next, requestCTX).response;
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
			}

			if (config.loglevel.requests) {
				syncout.log({
					'[' + requestID + ']',
					'(' + (config.loglevel.transportEvents ? contextID : conninfo.remoteAddr.hostname) + ')',
					next.method.toString(),
					next.url.pathname,
					"-->",
					functionResponse.has_value() ? functionResponse.value().status.code() : 101
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
