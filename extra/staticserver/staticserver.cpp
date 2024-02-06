#include "./staticserver.hpp"
#include "../../core/html/html.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTML;

StaticServer::StaticServer(const std::string& rootDir) : m_root(rootDir) {}

HTTP::Response StaticServer::serve(const HTTP::Request& request) const noexcept {
	return serve(request.url.pathname);
}

HTTP::Response StaticServer::serve(const std::string& pathname) const noexcept {

	//	return 404
	return Response(404, renderTemplate(Templates::servicePage, {
		{ "svcpage_statuscode", "404" },
		{ "svcpage_statustext", "Resource not found" },
		{ "svcpage_message_text", '"' + pathname + "\" could not be located" }
	}));
}
