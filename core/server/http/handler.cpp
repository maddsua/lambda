
#include "../../../lambda_build_options.hpp"
#include "../http.hpp"
#include "../../network/sysnetw.hpp"
#include "../../compression/compression.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../html/templates.hpp"
#include "../../json/json.hpp"

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
using namespace Lambda::Network;

HTTP::Response renderServerErrorPage(std::string message);
HTTP::Response composeServerErrorResponse(std::string message);

void HTTPServer::connectionHandler(Network::TCP::Connection&& conn, HTTPRequestCallback handlerCallback, const ServerConfig& config) noexcept {

	const auto& conninfo = conn.info();

	if (config.loglevel.connections) fprintf(stdout,
		"%s %s:%i connected on %i\n",
		Date().toHRTString().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);

	try {

		auto requestQueue = HTTPServer::HttpRequestQueue(conn, config.transport);

		while (requestQueue.await()) {

			auto nextRequest = requestQueue.next();
			auto responseDate = Date();

			time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			auto requestID = ShortID((timeHighres & ~0UL)).toString();

			Lambda::HTTP::Response response;
			std::optional<std::string> handlerError;

			try {

				response = handlerCallback(nextRequest.request, {
					requestID,
					conninfo,
					Console(requestID)
				});

			} catch(const std::exception& e) {

				if (config.loglevel.requests) {
					printf("%s %s crashed: %s\n", responseDate.toHRTString().c_str(), requestID.c_str(), e.what());
				}

				handlerError = e.what();

			} catch(...) {

				if (config.loglevel.requests) {
					printf("%s %s crashed: unhandled exception\n", responseDate.toHRTString().c_str(), requestID.c_str());
				}

				handlerError = "unhandled exception";
			}

			if (handlerError.has_value()) {
				response = config.servicePageType == ServicePageType::JSON ? 
					composeServerErrorResponse(handlerError.value()) :
					renderServerErrorPage(handlerError.value());
			}

			response.headers.set("date", responseDate.toUTCString());
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

			HTTPServer::writeResponse(response, conn, nextRequest.acceptsEncoding);

			if (config.loglevel.requests) {
				printf("%s[%s] (%s) %s %s --> %i\n",
					config.loglevel.timestamps ? (responseDate.toHRTString() + " ").c_str() : "",
					requestID.c_str(),
					conninfo.remoteAddr.hostname.c_str(),
					static_cast<std::string>(nextRequest.request.method).c_str(),
					nextRequest.pathname.c_str(),
					response.status.code()
				);
			}
		}

	} catch(const std::exception& e) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s [Service] Connection to %s terminated: %s\n",
			Date().toHRTString().c_str(),
			conninfo.remoteAddr.hostname.c_str(),
			e.what()
		);

	} catch(...) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s [Service] Connection to %s terminated (unknown error)\n",
			Date().toHRTString().c_str(),
			conninfo.remoteAddr.hostname.c_str()
		);
	}

	if (config.loglevel.connections) fprintf(stdout,
		"%s %s:%i disconnected from %i\n",
		Date().toHRTString().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);
}

HTTP::Response renderServerErrorPage(std::string message) {

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

HTTP::Response composeServerErrorResponse(std::string message) {

	JSON::Map responseObject = {
		{ "ok", false },
		{ "status", "failed" },
		{ "context", "function handler crashed" },
		{ "what", message }
	};

	return HTTP::Response(500, {
		{"content-type", "application/json"}
	}, JSON::stringify(responseObject));
}
