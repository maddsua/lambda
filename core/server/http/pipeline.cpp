
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

void Server::handleHttpRequest(Network::TCP::Connection& conn, const RequestQueueItem& next, HTTPRequestCallback handlerCallback, const ServerConfig& options, const ConnectionInfo& conninfo) {

	auto responseDate = Date();

	time_t timeHighres = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	auto requestID = ShortID((timeHighres & ~0UL)).toString();

	HTTP::Response response;

	try {

		response = handlerCallback(next.request, {
			requestID,
			conninfo,
			Console(requestID)
		});

	} catch(const std::exception& e) {

		if (options.loglevel.requests) {
			printf("%s %s crashed: %s\n", responseDate.toHRTString().c_str(), requestID.c_str(), e.what());
		}
		
		response = Server::errorResponse(500, std::string("Function handler crashed: ") + e.what());

	} catch(...) {

		if (options.loglevel.requests) {
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
	if (next.keepAlive) response.headers.set("connection", "keep-alive");
	if (!response.headers.has("content-type")) response.headers.set("content-type", "text/html; charset=utf-8");

	#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

		std::vector<uint8_t> responseBody;

		switch (next.acceptsEncoding) {

			case ContentEncodings::Brotli: {
				responseBody = Compress::brotliCompressBuffer(response.body.buffer(), Compress::Quality::Noice);
			} break;

			case ContentEncodings::Gzip: {
				responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Gzip);
			} break;

			case ContentEncodings::Deflate: {
				responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Defalte);
			} break;

			default: {
				responseBody = response.body.buffer();
			} break;
		}

		if (next.acceptsEncoding != ContentEncodings::None) {
			response.headers.set("content-encoding", contentEncodingMap.at(next.acceptsEncoding));
		}

	#else
		auto& responseBody = response.body.buffer();
	#endif

	auto bodySize = responseBody.size();
	response.headers.set("content-length", std::to_string(bodySize));

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : response.headers.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) conn.write(responseBody);

	if (options.loglevel.requests) {
		printf("%s[%s] (%s) %s %s --> %i\n",
			options.loglevel.timestamps ? (responseDate.toHRTString() + " ").c_str() : "",
			requestID.c_str(),
			conninfo.remoteAddr.hostname.c_str(),
			static_cast<std::string>(next.request.method).c_str(),
			next.request.url.pathname.c_str(),
			response.status.code()
		);
	}
}
