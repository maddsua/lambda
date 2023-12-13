#include "../server.hpp"

extern char _binary_core_server_resources_html_servicepage_html_start;
extern char _binary_core_server_resources_html_servicepage_html_end;

using namespace Lambda;

static const auto pageTemplate = std::string(&_binary_core_server_resources_html_servicepage_html_start, &_binary_core_server_resources_html_servicepage_html_end - &_binary_core_server_resources_html_servicepage_html_start);

typedef std::vector<std::pair<std::string, std::string>> TemplateContent;

std::string populateTemplate(const TemplateContent& content) {

	auto result = pageTemplate;
	auto matchIdx = std::string::npos;

	for (const auto& item : content) {
		while ((matchIdx = result.find(item.first)) != std::string::npos) {
			result.replace(matchIdx, item.first.size(), item.second);
		}
	}

	return result;
}

HTTP::Response serviceResponse(int statusCode, std::optional<std::string> errorMessage) {

	auto httpstatus = HTTP::Status(statusCode);

	TemplateContent content = {
		{ "${html_svcpage_statuscode}", std::to_string(statusCode) },
		{ "${html_svcpage_statustext}", httpstatus.text() },
		{ "${html_svcpage_message_text}", (errorMessage.has_value() ? errorMessage.value() : "That's all we know.") }
	};

	HTTP::Headers headers;
	headers.set("Content-Type", "text/html");

	return HTTP::Response(httpstatus, headers, populateTemplate(content));
}
