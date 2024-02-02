#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "../server.hpp"
#include "../http.hpp"
#include "../../network/tcp/connection.hpp"

namespace Lambda::Server::Handlers {

	void serverlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ServerlessCallback& handlerCallback) noexcept;
	void connectionHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;
};

#endif
