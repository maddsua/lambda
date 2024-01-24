#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__

#include "../network/tcp/connection.hpp"
#include "../network/tcp/listen.hpp"
#include "./server.hpp"
#include "./handlers.hpp"

#include <future>
#include <optional>
#include <queue>
#include <unordered_map>

namespace Lambda {

	struct RouteContext {
		Server::Handlers::HandlerFunction handler;
	};

	typedef std::unordered_map<std::string, RouteContext> Router;

	std::optional<RouteContext> matchRoute(const Router& router, const std::string& pathname);

};

#endif
