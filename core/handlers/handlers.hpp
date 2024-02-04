#ifndef __LIB_MADDSUA_LAMBDA_CORE_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_HANDLERS__

//	@todo: replace with options import
#include "../server/server.hpp"
#include "../network/tcp/connection.hpp"

#include <string>
#include <map>

namespace Lambda::Handlers {

	void serverlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ServerlessCallback& handlerCallback) noexcept;
	void connectionHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;

};

#endif
