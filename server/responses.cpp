#include "./server.hpp"

using namespace Lambda;

//	these variables (pointers, whatever, I'm too dumb for C++ anyway) should be imported from servicepage.html (servicepage.o)
extern char _binary_resources_html_servicepage_html_start;
extern char _binary_resources_html_servicepage_html_end;

static const auto template_start = &_binary_resources_html_servicepage_html_start;
static const auto template_end = &_binary_resources_html_servicepage_html_end;
static const auto pageTemplate = std::string(template_start, template_end - template_start);

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

HTTP::Response Lambda::serviceResponse(uint16_t httpStatus, const std::string& text) {

	TemplateContent content = {
		{ "${html_svcpage_statuscode}", std::to_string(httpStatus) },
		{ "${html_svcpage_statustext}", HTTP::statusText(httpStatus) },
		{ "${html_svcpage_message_text}", (text.size() ? text : "That's all we know.") }
	};

	return HTTP::Response(httpStatus, { { "Content-Type", "text/html" } }, populateTemplate(content));
}
