#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_ROUTER__

#include "./handlers.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace Lambda {

	struct RouteContext {
		Endpoints::HandlerFunction handler;
	};

	class Router {
		private:
			std::unordered_map<std::string, RouteContext> m_router;
			std::optional<RouteContext> staticHandler;
			std::vector<std::pair<std::string, std::string>> globRoutes;

		public:
			Router() {}
			Router(const RouteContext& routerInit);
			Router(const Endpoints::HandlerFunction& routerInit);
			Router(const std::initializer_list<std::pair<std::string, RouteContext>>& routerInit);
			Router& operator= (const Router& other) noexcept;
			Router& operator= (Router&& other) noexcept;
			std::optional<RouteContext> match(const std::string& pathname) const;
	};

};

#endif
