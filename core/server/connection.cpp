#include "./server.hpp"
#include "./internal.hpp"
#include "../http/http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;
using namespace Lambda::Server;
using namespace Lambda::Websocket;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

IncomingConnection::IncomingConnection(
	Network::TCP::Connection& connInit,
	const ServeOptions& optsInit
) : conn(connInit), opts(optsInit), ctx(conn, opts.transport) {}

std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot read next http request: connection protocol was changed");
	}

	try {

		const auto nextOpt = this->ctx.nextRequest();
		if (!nextOpt.has_value()) return std::nullopt;
		return nextOpt.value();

	} catch(const TransportError& err) {
		
		if (this->opts.loglevel.transportErrors) {
			syncout.error({ "HTTP TransportError:", err.message() });
		}

		if (err.action == TransportError::Action::Respond) {
			this->respond(Pages::renderErrorPage(400, err.message(), this->opts.errorResponseType));
		}

		this->conn.end();

	} catch(const std::exception& err) {
		
		if (this->opts.loglevel.transportErrors) {
			syncout.error({ "Connection error:", err.what() });
		}

		this->conn.end();
	}

	return std::nullopt;
}

void IncomingConnection::respond(const HTTP::Response& response) {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot send http response to a connection that had it's protocol changed");
	}

	this->ctx.respond(response);
}

WebsocketContext IncomingConnection::upgrateToWebsocket(const HTTP::Request& initialRequest) {

	auto headerUpgrade = Strings::toLowerCase(initialRequest.headers.get("Upgrade"));
	auto headerWsKey = initialRequest.headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size()) {
		throw std::runtime_error("Websocket initialization aborted: Invalid connection header");
	}

	if (this->ctx.hasPartialData()) {
		throw std::runtime_error("Websocket initialization aborted: Connection has unprocessed data");
	}

	auto combinedKey = headerWsKey + wsMagicString;

	auto keyHash = Crypto::SHA1().update(combinedKey).digest();

	auto handshakeReponse = HTTP::Response(101, {
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::toBase64(keyHash) }
	});

	this->respond(handshakeReponse);
	this->ctx.reset();

	this->activeProto = ActiveProtocol::WS;
	return WebsocketContext(this->conn, this->opts.transport);
}

WebsocketContext IncomingConnection::upgrateToWebsocket() {

	auto request = this->nextRequest();
	if (!request.has_value()) {
		throw std::runtime_error("Cannot establish websocket connection without handshake");
	}

	return this->upgrateToWebsocket(request.value());
}
