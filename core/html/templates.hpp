#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTML_TEMPLATES__
#define __LIB_MADDSUA_LAMBDA_CORE_HTML_TEMPLATES__

#include <string>
#include <map>

namespace Lambda::HTML {

	namespace Templates {
		const std::string& servicePage() noexcept;
	};

	typedef std::map<std::string, std::string> TemplateProps;
	std::string renderTemplate(const std::string& templateSource, const TemplateProps& props);

};

#endif
