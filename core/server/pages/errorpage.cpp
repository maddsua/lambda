
#include "../internal.hpp"
#include "../../http/http.hpp"
#include "../../json/json.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::Pages;
using namespace Lambda::Server::Pages::Templates;

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message) {
	return renderErrorPage(code, message, ErrorResponseType::HTML);
}

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type) {

	std::string pagecontent;

	if (type == ErrorResponseType::HTML) {

		auto templateSource = servicePage();

		pagecontent = renderTemplate(templateSource, {
			{ "svcpage_statuscode", std::to_string(code.code()) },
			{ "svcpage_statustext", "service error" },
			{ "svcpage_message_text", message }
		});

	} else {

		JSON::Map responseObject = {
			{ "ok", false },
			{ "status", "failed" },
			{ "context", "service error" },
			{ "what", message }
		};

		pagecontent = JSON::stringify(responseObject);
	}

	return Lambda::HTTP::Response(code, {
		{ "Content-Type", type == ErrorResponseType::HTML ? "text/html" : "application/json" }
	}, pagecontent);
}
