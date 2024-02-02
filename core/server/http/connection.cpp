#include "../server.hpp"
#include "../internal.hpp"
#include "../../http/http.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../encoding/encoding.hpp"
#include "../constants.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::HTTPTransport;

IncomingConnection::IncomingConnection(
	Network::TCP::Connection& conn,
	const HTTPTransportOptions& opts
) : ctx({ conn, opts, conn.info() }) {}


std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot read next http request: connection protocol was changed");
	}

	auto nextOpt = requestReader(this->ctx);
	if (!nextOpt.has_value()) return std::nullopt;
	auto& next = nextOpt.value();

	this->ctx.keepAlive = next.keepAlive;
	this->ctx.acceptsEncoding = next.acceptsEncoding;

	return next.request;
}

void IncomingConnection::respond(const HTTP::Response& response) {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot send http response to a connection that had it's protocol changed");
	}

	writeResponse(response, {
		this->ctx.acceptsEncoding,
		this->ctx.keepAlive,
		this->ctx.conn,
	});
}

WebsocketContext IncomingConnection::upgrateToWebsocket(const HTTP::Request& initialRequest) {

	auto headerUpgrade = Strings::toLowerCase(initialRequest.headers.get("Upgrade"));
	auto headerWsKey = initialRequest.headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size())
		throw std::runtime_error("Websocket initialization aborted: no valid handshake headers present");

	auto combinedKey = headerWsKey + wsMagicString;

	auto keyHash = Crypto::SHA1().update(combinedKey).digest();

	auto handshakeReponse = HTTP::Response(101, {
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::toBase64(keyHash) }
	});

	this->respond(handshakeReponse);

	this->activeProto = ActiveProtocol::WS;
	return WebsocketContext({
		this->ctx.conn,
		this->ctx.buffer
	});
}

WebsocketContext IncomingConnection::upgrateToWebsocket() {
	auto request = this->nextRequest();
	if (!request.has_value()) throw std::runtime_error("Cannot establish websocket connection without handshake");
	return this->upgrateToWebsocket(request.value());
}
