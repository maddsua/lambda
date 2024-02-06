
#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_STATIC_SERVER__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_STATIC_SERVER__

#include "../../core/http/http.hpp"

#include <string>
#include <optional>

namespace Lambda {

	class StaticServer {
		private:
			std::string m_root;

		public:
			StaticServer(const std::string& rootDir);

			HTTP::Response serve(const HTTP::Request& request) const noexcept;
			HTTP::Response serve(const std::string& pathname) const noexcept;

			std::string flattenPathName(std::string urlpath) const noexcept;
			std::optional<std::string> resolvePath(const std::string& pathname) const noexcept;
	};

};

#endif
