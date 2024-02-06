#include "./staticserver.hpp"
#include "../../core/html/html.hpp"

#include <filesystem>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTML;

StaticServer::StaticServer(const std::string& rootDir) : m_root(rootDir) {

	if (!std::filesystem::exists(this->m_root)) {
		throw std::runtime_error("StaticServer startup error: path \"" + this->m_root + "\" does not exist");
	}

	if (!std::filesystem::is_directory(this->m_root)) {
		throw std::runtime_error("StaticServer startup error: path \"" + this->m_root + "\" is not a directory");
	}
}

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
