#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "../network/tcp/connection.hpp"
#include "../network/tcp/listen.hpp"
#include "./server.hpp"

namespace Lambda::Server {

	void serveHTTP(Network::TCP::Connection&& conn, HandlerFunction handler, const ServeOptions& options);
	HTTP::Response errorResponse(int statusCode, std::optional<std::string> errorMessage);

};

#endif
