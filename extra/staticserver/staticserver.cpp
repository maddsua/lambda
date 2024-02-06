#include "./staticserver.hpp"
#include "../../core/html/html.hpp"

#include <filesystem>
#include <functional>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTML;

std::string StaticServer::flattenPathName(std::string urlpath) const noexcept {

	auto segmentPos = std::string::npos;
	while ((segmentPos = urlpath.find("\\")) != std::string::npos)	{
		urlpath.replace(segmentPos, 1, "/");
	}

	segmentPos = std::string::npos;
	while ((segmentPos = urlpath.find("../")) != std::string::npos)	{

		size_t segmentPrevStart = 0;
		size_t searchStart = segmentPos > 2 ? segmentPos - 2 : 0;
		for (size_t i = searchStart; i > 0; i--) {
			if (urlpath[i] == '/') {
				segmentPrevStart = i;
				break;
			}
		}

		urlpath.replace(segmentPrevStart, (segmentPos + 3) - segmentPrevStart, "/");
	}

	segmentPos = std::string::npos;
	while ((segmentPos = urlpath.find("./")) != std::string::npos)	{
		urlpath.replace(segmentPos, 1, "/");
	}

	segmentPos = std::string::npos;
	while ((segmentPos = urlpath.find("//")) != std::string::npos)	{
		urlpath.replace(segmentPos, 2, "/");
	}

	return urlpath;
}

StaticServer::StaticServer(const std::string& rootDir) {

	this->m_root = rootDir;

	if (this->m_root.ends_with('\\') || this->m_root.ends_with('/')) {
		this->m_root.erase(this->m_root.size() - 1);
	}

	if (!std::filesystem::exists(this->m_root)) {
		throw std::runtime_error("StaticServer startup error: path \"" + this->m_root + "\" does not exist");
	}

	if (!std::filesystem::is_directory(this->m_root)) {
		throw std::runtime_error("StaticServer startup error: path \"" + this->m_root + "\" is not a directory");
	}
}

std::optional<std::string> StaticServer::resolvePath(const std::string& pathname) const noexcept {

	auto basePath = this->m_root + (pathname.starts_with('/') ? pathname : ('/' + pathname));

	auto checkAvailability = [](const std::string& fspath) {
		return std::filesystem::exists(fspath) && std::filesystem::is_regular_file(fspath);
	};

	if (checkAvailability(basePath)) {
		return basePath;
	}

	//	drop unresolved files with extensions
	auto lastDotPos = basePath.find_last_of('.');
	auto lastSlashPos = basePath.find_last_of('/');
	if (lastDotPos != std::string::npos && (lastDotPos > lastSlashPos)) {
		return std::nullopt;
	}

	//	ensure the path ends with a slash
	if (!basePath.ends_with('/')) {
		basePath.push_back('/');
	}

	std::initializer_list<std::function<std::string()>> candidateList = {
		[&]() {
			return basePath + "index.html";
		},
		[&]() {
			return basePath + "index.htm";
		},
	};

	for (auto candidate : candidateList) {
		auto variant = candidate();
		if (checkAvailability(variant)) {
			return variant;
		}
	}

	return std::nullopt;
}

HTTP::Response StaticServer::serve(const HTTP::Request& request) const noexcept {
	return serve(request.url.pathname);
}

HTTP::Response StaticServer::serve(const std::string& pathname) const noexcept {

	auto normalizedPathname = flattenPathName(pathname);
	auto resolvedFile = this->resolvePath(normalizedPathname);

	if (resolvedFile.has_value()) {
		puts(resolvedFile.value().c_str());
		return Response(200, "found");
	}

	//	return 404
	return Response(404, renderTemplate(Templates::servicePage, {
		{ "svcpage_statuscode", "404" },
		{ "svcpage_statustext", "Resource not found" },
		{ "svcpage_message_text", '"' + pathname + "\" could not be located" }
	}));
}
