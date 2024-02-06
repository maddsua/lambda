#include "../server.hpp"
#include "../internal.hpp"
#include "../../http/http.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Server;

IncomingConnection::IncomingConnection(
	Network::TCP::Connection& connInit,
	const ServeOptions& optsInit
) : conn(connInit), opts(optsInit), ctx(conn, opts.transport) {}

std::optional<HTTP::Request> IncomingConnection::nextRequest() {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot read next http request: connection protocol was changed");
	}

	auto nextOpt = this->ctx.nextRequest();
	if (!nextOpt.has_value()) return std::nullopt;
	return nextOpt.value();
}

void IncomingConnection::respond(const HTTP::Response& response) {

	if (this->activeProto != ActiveProtocol::HTTP) {
		throw std::runtime_error("Cannot send http response to a connection that had it's protocol changed");
	}

	this->ctx.respond(response);
}
