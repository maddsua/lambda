#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;
using namespace Lambda::JSON;

int main(int argc, char const *argv[]) {

	auto handler = [&](const Request& req, const Context& context) {

		bool isFirstFisit = false;

		auto responseHeaders = HTTP::Headers();

		//	just setting a custom header
		responseHeaders.set("x-serverless", "true");
		responseHeaders.set("content-type", "application/json");

		//	check if user visited before by a cookie
		if (!req.cookies.has("userid")) {
			auto setCookies = HTTP::Cookies();
			setCookies.set("userid", "test_user_0");
			responseHeaders.set("Set-Cookie", setCookies.stringify());
			isFirstFisit = true;
		}

		//	get search query "user" param
		//	try opening url as http://localhost:8080/?user=maddsua
		auto username = req.url.searchParams.get("user");
		
		//	create response json
		JSON::Map testMap = {
			{"date", Date().toUTCString()},
			{"user", username.size() ? username : "anonymous"},
			{"useragent", req.headers.get("user-agent")},
			{"first_visit", isFirstFisit}
		};

		return HTTP::Response(200, responseHeaders, stringify(Property(testMap)));
	};

	ServerConfig initparams;
	initparams.loglevel.requests = true;
	auto server = ServerInstance(handler, initparams);
	server.awaitFinished();

	return 0;
}
