
#include "./handlers.hpp"
#include "../http/http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../pages/pages.hpp"
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
using namespace Lambda::Handlers;

void Handlers::serverlessHandler(
	Network::TCP::Connection&& conn,
	const ServeOptions& config,
	const ServerlessCallback& handlerCallback
) noexcept {

	const auto& conninfo = conn.info();

	auto createLogTimeStamp = [&]() {
		if (config.loglevel.timestamps) {
			return Date().toHRTString() + ' ';
		}
		return std::string();
	};

	if (config.loglevel.connections) fprintf(stdout,
		"%s%s:%i connected on %i\n",
		createLogTimeStamp().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);

	try {

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
					conninfo,
					Console(requestID, config.loglevel.timestamps)
				});

			} catch(const std::exception& e) {
				handlerError = e.what();
			} catch(...) {
				handlerError = "unhandled exception";
			}

			if (handlerError.has_value()) {

				response = Pages::renderErrorPage(500, handlerError.value(), config.errorResponseType);

				if (config.loglevel.requests) fprintf(stderr,
					"%s%s crashed: %s\n",
					createLogTimeStamp().c_str(),
					requestID.c_str(),
					handlerError.value().c_str()
				);
			}

			response.headers.set("x-request-id", requestID);

			connctx.respond(response);

			if (config.loglevel.requests) fprintf(stdout,
				"%s[%s] (%s) %s %s --> %i\n",
				createLogTimeStamp().c_str(),
				requestID.c_str(),
				conninfo.remoteAddr.hostname.c_str(),
				static_cast<std::string>(next.method).c_str(),
				next.url.pathname.c_str(),
				response.status.code()
			);
		}

	} catch(const std::exception& e) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s[Service] Connection to %s terminated: %s\n",
			createLogTimeStamp().c_str(),
			conninfo.remoteAddr.hostname.c_str(),
			e.what()
		);

	} catch(...) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s[Service] Connection to %s terminated (unknown error)\n",
			createLogTimeStamp().c_str(),
			conninfo.remoteAddr.hostname.c_str()
		);
	}

	if (config.loglevel.connections) fprintf(stdout,
		"%s%s:%i disconnected from %i\n",
		createLogTimeStamp().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);
}
