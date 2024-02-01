#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::HTTPServer {

	struct ReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
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

	struct WriterContext : HTTPTransportContext {
		Network::TCP::Connection& conn;
	};

	std::optional<IncomingRequest> requestReader(ReaderContext& ctx);
	void writeResponse(const HTTP::Response& response, const WriterContext& ctx);

	struct ConnectionContext : ReaderContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	};

	std::optional<std::pair<std::string, std::string>> parseBasicAuth(const std::string& header);

};

#endif
