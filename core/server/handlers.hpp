#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::Server::Handlers {

	void httpServerlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const HTTPRequestCallback& handlerCallback) noexcept;
	void httpExtendedHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;
};

#endif
