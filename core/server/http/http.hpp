#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__

#include "../server.hpp"
#include "../internal.hpp"
#include "../../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::HTTPServer {

	struct HTTPReaderContext : Server::ReaderContext {
		bool keepAlive = false;
	};

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	struct HTTPTransportContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	struct IncomingRequest : HTTPTransportContext {
		HTTP::Request request;
	};

	struct HTTPWriterContext : HTTPTransportContext {
		Network::TCP::Connection& conn;
	};

	std::optional<IncomingRequest> requestReader(HTTPReaderContext& ctx);
	void writeResponse(const HTTP::Response& response, const HTTPWriterContext& ctx);

	struct ConnectionContext : HTTPReaderContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	};

	std::optional<std::pair<std::string, std::string>> parseBasicAuth(const std::string& header);

};

#endif
