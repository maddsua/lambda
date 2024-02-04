
#include "./pages.hpp"
#include "../json/json.hpp"

using namespace Lambda;
using namespace Lambda::Pages;
using namespace Lambda::Pages::Templates;

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message) {
	return renderErrorPage(code, message, ErrorResponseType::HTML);
}

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type) {

	std::string pagecontent;

	if (type == ErrorResponseType::HTML) {

		pagecontent = renderTemplate(Templates::servicePage, {
			{ "svcpage_statuscode", std::to_string(code.code()) },
			{ "svcpage_statustext", code.text() },
			{ "svcpage_message_text", message }
		});

	} else {

		JSON::Map responseObject = {
			{ "ok", false },
			{ "status", "failed" },
			{ "context", code.text() },
			{ "what", message }
		};

		pagecontent = JSON::stringify(responseObject);
	}

	return Lambda::HTTP::Response(code, {
		{ "Content-Type", type == ErrorResponseType::HTML ? "text/html" : "application/json" }
	}, pagecontent);
}
