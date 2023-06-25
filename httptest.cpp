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

	return 0;
}