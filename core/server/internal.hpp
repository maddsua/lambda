#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <string>
#include <map>

namespace Lambda::Server {

	namespace Handlers {
		void serverlessHandler(Network::TCP::Connection& conn, const ServeOptions& config, const ServerlessCallback& handlerCallback);
		void streamHandler(Network::TCP::Connection& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback);
	};

	namespace Pages {
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type);
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message);
	};
};

#endif
