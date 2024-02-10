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

uint32_t hashConnectionData(const Network::Address& remoteAddr) {
	time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	size_t hash = std::hash<std::string>{}(remoteAddr.hostname + std::to_string(remoteAddr.port));
	return (timeHighres & ~0UL) ^ (timeHighres & (static_cast<size_t>(~0UL) << 32)) ^
		(hash & ~0UL) ^ (hash & (static_cast<size_t>(~0UL) << 32));
}

IncomingConnection::IncomingConnection(
	Network::TCP::Connection& connInit,
	const ServeOptions& optsInit
) : conn(connInit), opts(optsInit), ctx(conn, opts.transport),
	m_ctx_id(hashConnectionData(connInit.info().remoteAddr)) {}

const Crypto::ShortID& IncomingConnection::contextID() const noexcept {
	return this->m_ctx_id;
}

Network::TCP::Connection& IncomingConnection::tcpconn() const noexcept {
	return this->conn;
}

const Network::ConnectionInfo& IncomingConnection::conninfo() const noexcept {
	return this->conn.info();
}

std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot read next http request: connection protocol was changed");
	}

	try {

		const auto nextOpt = this->ctx.nextRequest();
		if (!nextOpt.has_value()) return std::nullopt;
		return nextOpt.value();

	} catch(const ProtocolError& err) {

		/*
			Look. It's not very pretty to rethrow an error but it's way better
			than coming up with	some elaborate structures that will provide a way
			to distinguish between different kinds of errors.
			Also most of the library uses exceptions to do error handling anyway
			so making any of that that would be just super inconsistent and confusing.
		*/
		if (err.respondStatus.has_value()) {
			this->respond(Pages::renderErrorPage(err.respondStatus.value(), err.message(), this->opts.errorResponseType));
		}

		throw;
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
