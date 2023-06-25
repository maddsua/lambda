#include "http/http.hpp"
#include <cstdio>

int main() {

	auto search = HTTP::URLSearchParams("tset=ok&gf=none");

	search.append("yeey", "yep yep");

	puts(search.stringify().c_str());

	/*auto split = HTTP::stringSplit(":", ":");
	printf("|%s|%s|\n", split.at(0).c_str(), split.at(1).c_str());*/

	return 0;
}