#include "../server.hpp"
#include "../handlers.hpp"
#include "../http.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::HTTPServer;
using namespace Lambda::Server::Handlers;

void Handlers::connectionHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept {
	auto connctx = IncomingConnection(&conn, config.transport);
	handlerCallback(connctx);
}

IncomingConnection::IncomingConnection(Network::TCP::Connection* conn, const HTTPTransportOptions& opts) {
	this->ctx = new ConnectionContext { *conn, opts, conn->info() };
}

IncomingConnection::~IncomingConnection() {
	delete this->ctx;
}

std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	auto nextOpt = requestReader(*this->ctx);
	if (!nextOpt.has_value()) return std::nullopt;
	auto& next = nextOpt.value();

	this->ctx->keepAlive = next.keepAlive;
	this->ctx->acceptsEncoding = next.acceptsEncoding;

	return next.request;
}

void IncomingConnection::respond(const HTTP::Response& response) {
	writeResponse(response, this->ctx->conn, this->ctx->acceptsEncoding);
}
