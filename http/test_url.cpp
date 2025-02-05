#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>

#include "./http.hpp"

#include "../utils/tests.hpp"

void test_container() {

	auto kv = Lambda::HTTP::Values();

	kv.set("Content-type ", "application/json");
	string_assert("content-type", kv.get(" content-type"), "application/json");

	kv.set(" Content-type ", "text/plain");
	string_assert("content-type", kv.get(" content-type"), "text/plain");

	kv.del("content-type");
	string_assert("content-type", kv.get(" content-typE "), "");
}

void test_search_parse_1() {

	auto search = Lambda::URLSearchParams("&&=&user=maddsua&age=unknown&location=earth");

	string_assert("user", search.get(" \n user "), "maddsua");
	string_assert("age", search.get("AgE"), "unknown");
	string_assert("location", search.get("LOCATION"), "earth");
}

void test_search_serialize_1() {

	auto search = Lambda::URLSearchParams();
	search.set(" user", "maddsua");
	search.set("OS", "linux");
	search.set("  ", "test");
	search.set("auth", "type=token");

	string_assert("to_string", search.to_string(), "auth=type%3Dtoken&os=linux&user=maddsua");
}

void test_url_parse_1() {

	auto url = Lambda::URL("https://maddsua:test@localhost:8080/store?location=earth&query=miata#top");

	string_assert("scheme", url.scheme, "https");
	string_assert("host", url.host, "localhost:8080");
	string_assert("path", url.path, "/store");
	string_assert("search", url.search.to_string(), "location=earth&query=miata");
	string_assert("fragment", url.fragment, "#top");

	if (!url.user.has_value()) {
		throw std::logic_error("user credentials are not defined");
	}

	string_assert("user.user", url.user.value().user, "maddsua");
	string_assert("user.password", url.user.value().password, "test");
}

void test_url_parse_2() {

	auto url = Lambda::URL("maddsua:test@localhost:8080/store?location=earth&query=miata#");

	string_assert("scheme", url.scheme, "");
	string_assert("host", url.host, "localhost:8080");
	string_assert("path", url.path, "/store");
	string_assert("search", url.search.to_string(), "location=earth&query=miata");
	string_assert("fragment", url.fragment, "#");

	if (!url.user.has_value()) {
		throw std::logic_error("user credentials are not defined");
	}

	string_assert("user.user", url.user.value().user, "maddsua");
	string_assert("user.password", url.user.value().password, "test");
}

void test_url_parse_3() {

	auto url = Lambda::URL("localhost:8080/store?");

	string_assert("scheme", url.scheme, "");
	string_assert("host", url.host, "localhost:8080");
	string_assert("path", url.path, "/store");
	string_assert("search", url.search.to_string(), "");
	string_assert("fragment", url.fragment, "");

	if (url.user.has_value()) {
		throw std::logic_error("user credentials must not bt defined");
	}
}

void test_url_parse_4() {

	auto url = Lambda::URL("http://maddsua:@localhost/api");

	string_assert("scheme", url.scheme, "http");
	string_assert("host", url.host, "localhost");
	string_assert("path", url.path, "/api");
	string_assert("search", url.search.to_string(), "");
	string_assert("fragment", url.fragment, "");

	if (!url.user.has_value()) {
		throw std::logic_error("user credentials are not defined");
	}

	string_assert("user.user", url.user.value().user, "maddsua");
	string_assert("user.password", url.user.value().password, "");
}

void test_url_parse_5() {

	auto url = Lambda::URL("http://maddsua@localhost/api");

	string_assert("scheme", url.scheme, "http");
	string_assert("host", url.host, "localhost");
	string_assert("path", url.path, "/api");
	string_assert("search", url.search.to_string(), "");
	string_assert("fragment", url.fragment, "");

	if (!url.user.has_value()) {
		throw std::logic_error("user credentials are not defined");
	}

	string_assert("user.user", url.user.value().user, "maddsua");
	string_assert("user.password", url.user.value().password, "");
}

void test_url_parse_6() {

	auto url = Lambda::URL("http://:password@localhost/api");

	string_assert("scheme", url.scheme, "http");
	string_assert("host", url.host, "localhost");
	string_assert("path", url.path, "/api");
	string_assert("search", url.search.to_string(), "");
	string_assert("fragment", url.fragment, "");

	if (url.user.has_value()) {
		throw std::logic_error("user credentials must not bt defined");
	}
}

void test_url_serialize_1() {

	Lambda::URL url;
	url.user = Lambda::BasicAuth{ "maddsua", "test" };
	url.scheme = "https";
	url.host = "localhost:8080";
	url.path = "/store";
	url.search = Lambda::URLSearchParams("query=miata&location=earth");

	string_assert("to_string", url.to_string(), "https://maddsua:test@localhost:8080/store?location=earth&query=miata");
}

int main() {

	auto queue = TestQueue({
		{ "container_1", test_container },
		{ "search_parse_1", test_search_parse_1 },
		{ "search_serialize_1", test_search_serialize_1 },
		{ "url_parse_1", test_url_parse_1 },
		{ "url_parse_2", test_url_parse_2 },
		{ "url_parse_3", test_url_parse_3 },
		{ "url_parse_4", test_url_parse_4 },
		{ "url_parse_5", test_url_parse_5 },
		{ "url_parse_6", test_url_parse_6 },
		{ "url_serialize_1", test_url_serialize_1 }
	});

	return exec_queue(queue);
}
