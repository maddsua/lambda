#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_IMPL__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_IMPL__

#include "./server.hpp"
#include "../http/transport.hpp"
#include "../network/tcp/connection.hpp"

#include <string>
#include <map>

namespace Lambda::Server {

	enum struct HandlerMode {
		HTTP, WS, SSE
	};

	void connectionHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ServerlessCallback& handlerCallback) noexcept;

	namespace Pages {
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type);
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message);
	};
};

#endif
