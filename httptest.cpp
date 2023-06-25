#include "http/http.hpp"
#include <cstdio>

int main() {

	auto search = HTTP::URLSearchParams("tset=ok&gf=none");

	search.append("yeey", "yep yep");

	puts(search.stringify().c_str());

	auto header = HTTP::Headers();
	header.set("content-type", "application/test");
	header.set("auTHoRiZaTiOn", "Bearer idX");

	puts(header.stringify().c_str());

	auto response = HTTP::Response(204, {
		{"contenty-type", "text"}
	}, "woohooo");

	auto respdump = response.dump();

	puts(std::string(respdump.begin(), respdump.end()).c_str());

	auto url = HTTP::URL("https://google.com/kittens?test=22");

	url.searchParams.set("new", "supernew");

	puts(url.href().c_str());

	return 0;
}