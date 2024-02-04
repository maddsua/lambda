
#include "../internal.hpp"
#include "../../http/http.hpp"
#include "../../html/templates.hpp"
#include "../../json/json.hpp"

using namespace Lambda;
using namespace Lambda::Server;

HTTP::Response Servicepage::renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type) {

	std::string pagecontent;

	if (type == ErrorResponseType::HTML) {

		auto templateSource = HTML::Templates::servicePage();

		pagecontent = HTML::renderTemplate(templateSource, {
			{ "svcpage_statuscode", std::to_string(code.code()) },
			{ "svcpage_statustext", "service error" },
			{ "svcpage_message_text", "Function handler crashed: " + message }
		});

	} else {

		JSON::Map responseObject = {
			{ "ok", false },
			{ "status", "failed" },
			{ "context", "function handler crashed" },
			{ "what", message }
		};

		pagecontent = JSON::stringify(responseObject);
	}

	return Lambda::HTTP::Response(code, {
		{ "Content-Type", type == ErrorResponseType::HTML ? "text/html" : "application/json" }
	}, pagecontent);
}
