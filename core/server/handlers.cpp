
#include "./internal.hpp"
#include "../http/http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../json/json.hpp"
#include "../crypto/crypto.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>
#include <map>
#include <set>
#include <optional>

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::Handlers;

void Handlers::serverlessHandler(
	Network::TCP::Connection& conn,
	const ServeOptions& config,
	const ServerlessCallback& handlerCallback
) {

	const auto& conninfo = conn.info();
	auto connctx = IncomingConnection(conn, config);

	while (auto nextOpt = connctx.nextRequest()){

		if (!nextOpt.has_value()) break;

		auto& next = nextOpt.value();
		auto requestID = Crypto::ShortID().toString();

		HTTP::Response response;
		std::optional<std::string> handlerError;

		try {

			response = handlerCallback(next, {
				requestID,
				conninfo
			});

		} catch(const std::exception& e) {
			handlerError = e.what();
		} catch(...) {
			handlerError = "unhandled exception";
		}

		if (handlerError.has_value()) {

			response = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);

			if (config.loglevel.requests) {
				syncout.error({ requestID, "crashed:", handlerError.value() });
			}
		}

		response.headers.set("x-request-id", requestID);

		connctx.respond(response);

		if (config.loglevel.requests) {
			syncout.log({
				'[' + requestID + ']',
				'(' + conninfo.remoteAddr.hostname + ')',
				next.method.toString(),
				next.url.pathname,
				"-->",
				response.status.code()
			});
		}
	}
}

void Handlers::streamHandler(
	Network::TCP::Connection& conn,
	const ServeOptions& config,
	const ConnectionCallback& handlerCallback
) {

	auto connctx = IncomingConnection(conn, config);
	std::optional<std::string> handlerError;

	try {

		handlerCallback(connctx);

	} catch(const std::exception& e) {
		handlerError = e.what();
	} catch(...) {
		handlerError = "unhandled exception";
	}

	if (handlerError.has_value()) {

		if (config.loglevel.requests) {
			syncout.error({ "tcp handler crashed:", handlerError.value() });
		}

		auto errorResponse = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);
		connctx.respond(errorResponse);
	}
}
