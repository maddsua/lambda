#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__

#include "./handlers.hpp"

#include <optional>
#include <unordered_map>

namespace Lambda {

	struct RouteContext {
		Server::Handlers::HandlerFunction handler;
	};

	class Router {
		private:
			std::unordered_map<std::string, RouteContext> m_router;
		public:
			Router() {}
			Router(const RouteContext& routerInit);
			Router(const Server::Handlers::HandlerFunction& routerInit);
			Router(const std::initializer_list<std::pair<std::string, RouteContext>>& routerInit);
			Router& operator= (const Router& other) noexcept;
			Router& operator= (Router&& other) noexcept;
			std::optional<RouteContext> match(const std::string& pathname) const;
	};

};

#endif
