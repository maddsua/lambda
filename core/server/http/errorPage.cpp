#include "../http.hpp"

extern char _binary_core_html_resources_servicepage_html_start;
extern char _binary_core_html_resources_servicepage_html_end;

using namespace Lambda;

static const auto pageTemplate = std::string(&_binary_core_html_resources_servicepage_html_start, &_binary_core_html_resources_servicepage_html_end - &_binary_core_html_resources_servicepage_html_start);

HTTP::Response HTTPServer::errorResponse(uint32_t statusCode, std::optional<std::string> errorMessage) {

	auto httpstatus = Lambda::HTTP::Status(statusCode);

	auto pagehtml = HTML::renderTemplate(pageTemplate, {
		{ "svcpage_statuscode", std::to_string(statusCode) },
		{ "svcpage_statustext", httpstatus.text() },
		{ "svcpage_message_text", (errorMessage.has_value() ? errorMessage.value() : "That's all we know.") }
	});

	return Lambda::HTTP::Response(httpstatus, {
		{ "Content-Type", "text/html" }
	}, pagehtml);
}
