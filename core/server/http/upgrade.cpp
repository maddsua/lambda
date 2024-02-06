#include "../server.hpp"
#include "../../http/http.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../encoding/encoding.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

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
