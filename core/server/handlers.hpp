#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "./handlers/console.hpp"

#include <functional>

namespace Lambda::Server::Handlers {

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HandlerFunction;

};

#endif
