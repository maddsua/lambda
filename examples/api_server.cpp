#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {

		auto responseHeaders = HTTP::Headers();

		//	just setting a custom header
		responseHeaders.set("x-serverless", "true");
		responseHeaders.set("content-type", "application/json");

		auto url = req.unwrapURL();
		auto cookies = req.getCookies();

		//	get search query "user" param
		//	try opening url as http://localhost:8080/?user=maddsua
		auto username = url.searchParams.get("user");

		//	check if user visited before by a cookie
		bool isFirstFisit = !cookies.has("userid");

		//	create response json
		JSON::Map testMap = {
			{"date", Date().toUTCString()},
			{"user", username.size() ? username : "anonymous"},
			{"useragent", req.headers.get("user-agent")},
			{"first_visit", isFirstFisit}
		};

		auto response = HTTP::Response(200, responseHeaders, stringify(Property(testMap)));

		//	set a user id cookie to check for on next request
		if (isFirstFisit) {
			auto newCookies = HTTP::Cookies();
			newCookies.set("userid", "test_user_0");
			newCookies.set("x_lambda", "control", {
				"Secure",
				{ "expires", "23 Oct 2077 08:28:00 GMT" }
			});
			response.setCookies(newCookies);
		}

		return response;
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
