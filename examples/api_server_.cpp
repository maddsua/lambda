/**
 * In this simple demo we return some json to the client
 * Check the code below to find out more!
*/

#include <iostream>

#include "../lambda.hpp"

using namespace Lambda;

HTTP::Response callbackServerless(Request& request, Context& context) {

	puts(("Request to \"" + request.url.pathname + "\" from " + context.clientIP).c_str());

	auto response = Response();

	//	just setting a custom header
	response.headers.set("x-serverless", "true");

	//	get search query "user" param
	//	try opening url as http://localhost:8080/?user=maddsua
	auto username = request.url.searchParams.get("user");
	
	//	check if user visited before by a cookie
	auto cookies = Cookies(request);
	bool isFirstFisit = false;

	if (!cookies.has("userid")) {
		auto newCookies = Cookies();
		newCookies.set("userid", "test_user_0");
		response.headers.set("Set-Cookie", newCookies.stringify());
		isFirstFisit = true;
	}

	//	create response json
	auto responseBody = JSON_Object();

	responseBody.addString("timestamp", serverDate());
	responseBody.addString("user", username.size() ? username : "anonymous");
	responseBody.addString("useragent", request.headers.get("user-agent"));
	responseBody.addBool("first_visit", isFirstFisit);

	response.setText(responseBody.stringify());
	response.headers.append("content-type", "application/json");

	return response;
};

int main() {

	auto handler = [&](const Request& req, const Context& context) {

		auto console = context.console;

		//	get record key
		auto key = req.url.searchParams.get("record");
		if (!key.size()) return HTTP::Response(HTTP::Status(400), "no record key provided");

		switch (req.method) {

			case HTTP::Methods::GET: {
				return storage.hasItem(key) ?
					HTTP::Response(storage.getItem(key)) :
					HTTP::Response(HTTP::Status(404), "object not found");
			} break;

			case HTTP::Methods::POST: {
				storage.setItem(key, req.body.text());
				return HTTP::Response(202, "created");
			} break;

			case HTTP::Methods::DEL: {
				storage.removeItem(key);
				return HTTP::Response(200, "deleted");
			} break;
			
			default: break;
		}

		return HTTP::Response(HTTP::Status(400), "bad request");
	};

	HttpServerConfig initparams;
	initparams.loglevel.logRequests = true;
	auto server = HttpServer(handler, initparams);
	server.awaitFinished();

	return 0;
}
