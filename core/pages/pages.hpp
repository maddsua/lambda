#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVICE_PAGES__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVICE_PAGES__

//	@todo: replace with options import
#include "../server/server.hpp"
#include "../http/http.hpp"

#include <string>
#include <map>

namespace Lambda::Pages {

	namespace Templates {
		extern const std::string servicePage;
	};

	typedef std::map<std::string, std::string> TemplateProps;
	std::string renderTemplate(const std::string& templateSource, const TemplateProps& props);

	HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type);
	HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message);
};

#endif
