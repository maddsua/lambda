#include "../server.hpp"
#include "../http.hpp"
#include "../handlers/handlers.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::HTTPServer;
using namespace Lambda::WSServer;

IncomingConnection::IncomingConnection(Network::TCP::Connection* conn, const HTTPTransportOptions& opts) {
	this->ctx = new ConnectionContext { *conn, opts, conn->info() };
}

IncomingConnection::~IncomingConnection() {
	delete this->ctx;
}

std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot read next http request: connection protocol was changed");
	}

	auto nextOpt = requestReader(*this->ctx);
	if (!nextOpt.has_value()) return std::nullopt;
	auto& next = nextOpt.value();

	this->ctx->keepAlive = next.keepAlive;
	this->ctx->acceptsEncoding = next.acceptsEncoding;

	return next.request;
}

void IncomingConnection::respond(const HTTP::Response& response) {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot send http response to a connection that had it's protocol changed");
	}

	writeResponse(response, {
		this->ctx->acceptsEncoding,
		this->ctx->keepAlive,
		this->ctx->conn,
	});
}


WebsocketContext IncomingConnection::upgrateToWebsocket() {
	this->activeProto = ActiveProtocol::WS;
	return WebsocketContext(this->ctx);
}
