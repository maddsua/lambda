#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "../network/tcp/connection.hpp"
#include "../network/tcp/listen.hpp"
#include "./server.hpp"

namespace Lambda::Server {

	enum struct ContentEncodings {
		None = 0,
		Brotli = 1,
		Gzip = 2,
		Deflate = 3,
	};

	struct PipelineItem {
		HTTP::Request request;
		uint32_t id;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	void httpPipeline(Network::TCP::Connection&& conn, HandlerFunction handler, const ServeOptions& options);
	HTTP::Response errorResponse(int statusCode, std::optional<std::string> errorMessage);

};

#endif
