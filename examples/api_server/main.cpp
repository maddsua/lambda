#include <iostream>

#include "../../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [](const Request& req, const Context& context) {

		//	get search query "user" param
		//	try opening url as http://localhost:8180/?user=maddsua
		auto username = req.url.searchParams.get("user");

		//	check if user visited before by a cookie
		auto cookies = req.getCookies();
		bool isFirstFisit = !cookies.has("userid");

		//	create response json
		JSON::Map testMap = {
			{"date", Date().toUTCString()},
			{"user", username.size() ? username : "anonymous"},
			{"useragent", req.headers.get("user-agent")},
			{"first_visit", isFirstFisit}
		};

		auto response = HTTP::Response(200, {
			{"x-serverless", "true"},
			{"content-type", "application/json"}
		}, stringify(Property(testMap)));

		//	set a user id cookie to check for on next request
		if (isFirstFisit) {
			auto newCookies = HTTP::Cookies();
			newCookies.set("userid", "test_user_0");
			newCookies.set("x_lambda", "control", {
				"Secure",
				{"expires", "23 Oct 2077 08:28:00 GMT"}
			});
			response.setCookies(newCookies);
		}

		return response;
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = LambdaInstance(handler, initparams);

	server.awaitFinished();

	return 0;
}
