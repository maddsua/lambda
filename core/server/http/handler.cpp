
#include "../http.hpp"
#include "../../network/sysnetw.hpp"
#include "../../compression/compression.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../../lambda_build_options.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>
#include <map>
#include <set>

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Network;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

void Server::connectionHandler(Network::TCP::Connection&& conn, HTTPRequestCallback handlerCallback, const ServerConfig& config) noexcept {

	const auto& conninfo = conn.info();

	if (config.loglevel.connections) fprintf(stdout,
		"%s %s:%i connected on %i\n",
		Date().toHRTString().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);

	try {

		auto requestQueue = Server::HttpRequestQueue(conn, config.transport);

		while (requestQueue.await()) {

			auto nextRequest = requestQueue.next();
			auto responseDate = Date();

			time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			auto requestID = ShortID((timeHighres & ~0UL)).toString();

			HTTP::Response response;

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
				
				response = Server::errorResponse(500, std::string("Function handler crashed: ") + e.what());

			} catch(...) {

				if (config.loglevel.requests) {
					printf("%s %s crashed: unhandled exception\n", responseDate.toHRTString().c_str(), requestID.c_str());
				}

				response = Server::errorResponse(500, "Function handler crashed: unhandled exception");
			}

			if (response.setCookies.size()) {
				response.headers.set("Set-Cookie", response.setCookies.stringify());
			}

			response.headers.set("date", responseDate.toUTCString());
			response.headers.set("server", "maddsua/lambda");
			response.headers.set("x-request-id", requestID);
			if (nextRequest.keepAlive) response.headers.set("connection", "keep-alive");
			if (!response.headers.has("content-type")) response.headers.set("content-type", "text/html; charset=utf-8");

			if (config.loglevel.requests) {
				printf("%s[%s] (%s) %s %s --> %i\n",
					config.loglevel.timestamps ? (responseDate.toHRTString() + " ").c_str() : "",
					requestID.c_str(),
					conninfo.remoteAddr.hostname.c_str(),
					static_cast<std::string>(nextRequest.request.method).c_str(),
					nextRequest.request.url.pathname.c_str(),
					response.status.code()
				);
			}

			Server::writeHttpResponse(response, conn, nextRequest.acceptsEncoding);
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
