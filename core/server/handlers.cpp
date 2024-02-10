
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

uint32_t shorthashIpAddressString(const std::string& ipAddress) {

	union {
		int32_t i32 = 0;
		uint8_t buff[4];
	} id;

	for (size_t i = 0; i < ipAddress.size(); i++) {
		if (ipAddress[i] == '.' || ipAddress[i] == ':') continue;
		id.buff[i % 4] ^= ipAddress[i];
	}

	return id.i32;
}

void Handlers::serverlessHandler(
	IncomingConnection& connctx,
	const ServeOptions& config,
	const ServerlessCallback& handlerCallback
) {

	const auto& conninfo = connctx.conninfo();

	while (auto nextOpt = connctx.nextRequest()){

		if (!nextOpt.has_value()) break;

		const auto& next = nextOpt.value();
		const auto& requestID = Crypto::ShortID().toString() + '-' + connctx.contextID().toString() + '-' + Crypto::ShortID(shorthashIpAddressString(conninfo.remoteAddr.hostname + std::to_string(conninfo.remoteAddr.port))).toString();

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

			if (config.loglevel.requests) {
				syncout.error({ requestID, "crashed:", handlerError.value() });
			}

			response = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);
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
	IncomingConnection& connctx,
	const ServeOptions& config,
	const ConnectionCallback& handlerCallback
) {

	std::optional<std::string> handlerError;

	try {
		handlerCallback(connctx);
	} catch(const std::exception& e) {
		handlerError = e.what();
	} catch(...) {
		handlerError = "unhandled exception";
	}

	if (handlerError.has_value()) {

		if (config.loglevel.requests || config.loglevel.connections) {
			syncout.error({
				"[Transport] streamHandler crashed in",
				connctx.contextID().toString() + ":",
				handlerError.value()
			});
		}

		auto errorResponse = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);
		connctx.respond(errorResponse);
	}
}
