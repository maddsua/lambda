
#include "./server_impl.hpp"
#include "../http/http.hpp"
#include "../json/json.hpp"
#include "../html/html.hpp"

using namespace Lambda;
using namespace Lambda::HTML;
using namespace Lambda::Server;
using namespace Lambda::Server::Pages;

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message) {
	return renderErrorPage(code, message, ErrorResponseType::HTML);
}

HTTP::Response Pages::renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type) {

	HTTP::Response errorResponse { code };

	switch (type) {

		case ErrorResponseType::HTML: {
			errorResponse.body = renderTemplate(Templates::servicePage, {
				{ "svcpage_statuscode", std::to_string(code.code()) },
				{ "svcpage_statustext", code.text() },
				{ "svcpage_message_text", message }
			});
			errorResponse.headers.set("content-type", "text/html");
		} break;

		case ErrorResponseType::JSON: {
			errorResponse.body = JSON::stringify(JSON::Map({
				{ "ok", false },
				{ "status", "failed" },
				{ "context", code.text() },
				{ "what", message }
			}));
			errorResponse.headers.set("content-type", "application/json");
		} break;
		
		default: {
			errorResponse.body = "Backend error: " + message + "\r\nmaddsua/lambda\r\n";
			errorResponse.headers.set("content-type", "text/plain");
		} break;
	}

	return errorResponse;
}
