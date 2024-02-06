#include "./staticserver.hpp"
#include "../../core/html/html.hpp"
#include "../../core/polyfill/polyfill.hpp"

#include <filesystem>
#include <functional>
#include <fstream>
#include <vector>
#include <cstdint>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTML;

std::vector<uint8_t> readBinaryFileSync(const std::string& filepath) {
	auto readablefile = std::fstream(filepath, std::ios::in | std::ios::binary);
	if (!readablefile.is_open()) return {};
	return std::vector<uint8_t>(std::istreambuf_iterator<char>(readablefile), {});
}

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

	//	return 404
	if (!resolvedFile.has_value()) {
		return Response(404, renderTemplate(Templates::servicePage, {
			{ "svcpage_statuscode", "404" },
			{ "svcpage_statustext", "Resource not found" },
			{ "svcpage_message_text", '"' + pathname + "\" could not be located" }
		}));
	}

	auto& resolvedFilePath = resolvedFile.value();
	auto extension = std::filesystem::relative(resolvedFilePath).extension();
	auto mimetype = Content::getExtMimetype(extension.generic_string());

	auto fileContent = readBinaryFileSync(resolvedFilePath);

	return Response(200, {
		{ "content-type", mimetype }
	}, fileContent);
}
