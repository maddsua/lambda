#include "../server.hpp"
#include "../internal.hpp"
#include "../../http/http.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::HTTPTransport;

IncomingConnection::IncomingConnection(
	Network::TCP::Connection& conn,
	const ServeOptions& opts
) : ctx({
	conn,
	opts.transport,
	conn.info(),
	opts.errorResponseType
}) {}

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
