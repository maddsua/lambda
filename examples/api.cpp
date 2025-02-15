//	A sample REST API server

#include <cstdio>

#include "../lambda.hpp"
#include "../http/http_utils.hpp"

using namespace Lambda;

void handler_fn(Request& req, ResponseWriter& wrt) {

	Log::log("--> [{}] {} {} {{}}", {
		req.remote_addr.hostname,
		HTTP::method_to_string(req.method),
		req.url.to_string(),
		req.body.text()
	});

	if (req.url.path != "/api") {
		wrt.write_header(Status::NotFound);
		return;
	}
	
	//	get search query "user" param
	//	try opening url as http://localhost:8180/?user=maddsua
	auto username = req.url.search.get("user");

	//	check if user visited before by a cookie
	bool is_first_visit = !req.cookies.has("userid");
	if (is_first_visit) {
		wrt.set_cookie(Cookie {
			.name = "userid",
			.value = "test_user_x",
			.secure = true,
			.http_only = true,
		});
	}

	//	create response json
	JSON::Object api_response = {
		{ "date", Date().to_utc_string() },
		{ "user", username.size() ? username : "anonymous" },
		{ "useragent", req.headers.get("user-agent") },
		{ "first_visit", is_first_visit }
	};

	wrt.header().set("content-type", "application/json");
	wrt.write(JSON::stringify(api_response));
};

int main() {
	auto server = Lambda::Server(handler_fn, { .debug = true });
	server.serve();
	return 0;
}
