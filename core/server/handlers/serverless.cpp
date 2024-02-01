
#include "../handlers.hpp"
#include "../../http/http.hpp"
#include "../../../build_options.hpp"
#include "../../compression/compression.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../html/templates.hpp"
#include "../../json/json.hpp"
#include "../../crypto/crypto.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>
#include <map>
#include <set>
#include <optional>

using namespace Lambda;
using namespace Lambda::HTTPServer;
using namespace Lambda::Server;
using namespace Lambda::Server::Handlers;
using namespace Lambda::Network;

Lambda::HTTP::Response renderServerErrorPage(std::string message);
Lambda::HTTP::Response composeServerErrorResponse(std::string message);

void Handlers::httpServerlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const HTTPRequestCallback& handlerCallback) noexcept {

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

		auto requestQueue = HttpRequestQueue(conn, config.transport);

		while (requestQueue.await()) {

			auto requestID = Crypto::ShortID().toString();

			auto nextRequest = requestQueue.next();
			Lambda::HTTP::Response response;
			std::optional<std::string> handlerError;

			try {

				response = handlerCallback(nextRequest.request, {
					requestID,
					conninfo,
					Console(requestID, config.loglevel.timestamps)
				});

			} catch(const std::exception& e) {

				if (config.loglevel.requests) fprintf(stderr,
					"%s%s crashed: %s\n",
					createLogTimeStamp().c_str(),
					requestID.c_str(),
					e.what()
				);

				handlerError = e.what();

			} catch(...) {

				if (config.loglevel.requests) fprintf(stderr,
					"%s%s crashed: unhandled exception\n",
					createLogTimeStamp().c_str(),
					requestID.c_str()
				);

				handlerError = "unhandled exception";
			}

			if (handlerError.has_value()) {
				response = config.errorResponseType == ErrorResponseType::JSON ? 
					composeServerErrorResponse(handlerError.value()) :
					renderServerErrorPage(handlerError.value());
			}

			response.headers.set("date", Date().toUTCString());
			response.headers.set("server", "maddsua/lambda");
			response.headers.set("x-request-id", requestID);

			//	set connection header to acknowledge keep-alive mode
			if (nextRequest.keepAlive) {
				response.headers.set("connection", "keep-alive");
			}

			//	set content type in case it's not provided in response
			if (!response.headers.has("content-type")) {
				response.headers.set("content-type", "text/html; charset=utf-8");
			}

			Handlers::writeResponse(response, conn, nextRequest.acceptsEncoding);

			if (config.loglevel.requests) fprintf(stdout,
				"%s[%s] (%s) %s %s --> %i\n",
				createLogTimeStamp().c_str(),
				requestID.c_str(),
				conninfo.remoteAddr.hostname.c_str(),
				static_cast<std::string>(nextRequest.request.method).c_str(),
				nextRequest.pathname.c_str(),
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

Lambda::HTTP::Response renderServerErrorPage(std::string message) {

	auto templateSource = HTML::Templates::servicePage();

	auto pagehtml = HTML::renderTemplate(templateSource, {
		{ "svcpage_statuscode", std::to_string(500) },
		{ "svcpage_statustext", "service error" },
		{ "svcpage_message_text", "Function handler crashed: " + message }
	});

	return Lambda::HTTP::Response(500, {
		{ "Content-Type", "text/html" }
	}, pagehtml);
}

Lambda::HTTP::Response composeServerErrorResponse(std::string message) {

	JSON::Map responseObject = {
		{ "ok", false },
		{ "status", "failed" },
		{ "context", "function handler crashed" },
		{ "what", message }
	};

	return Lambda::HTTP::Response(500, {
		{"content-type", "application/json"}
	}, JSON::stringify(responseObject));
}
