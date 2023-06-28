#include "../http/http.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	std::cout << "\r\n--- HTTP component test begin --- \r\n\r\n";

	//	create simple search query
	auto searchQuery = HTTP::URLSearchParams("userid=0007&hl=en");
	searchQuery.append("state", "ua");
	searchQuery.set("userid", "0008");
	std::cout << "Search query entry \"hl\": " << searchQuery.get("hl") << std::endl;
	std::cout << "Complete search query: " << searchQuery.stringify() << std::endl;
	std::cout << std::endl;

	//	create headers object
	auto header = HTTP::Headers();
	header.set("content-type", "application/json");
	header.set("auTHoRiZaTiOn", "Bearer token-xxxxxx");
	std::cout << header.stringify() << std::endl;
	std::cout << std::endl;

	//	create url object
	auto url = HTTP::URL("/api");
	url.searchParams = searchQuery;
	url.searchParams.set("session", "new");
	std::cout << url.href() << std::endl;
	std::cout << std::endl;

	//	create request object
	auto reqtext = std::string("GET /test HTTP/1.1\r\nTest-header    :    lambda\r\n\r\n");
	auto reqbin = std::vector<uint8_t>(reqtext.begin(), reqtext.end());
	auto req = HTTP::Request(reqbin);
	std::cout << req.method << ":" << req.path << std::endl;
	std::cout << req.searchParams.stringify() << std::endl;
	std::cout << std::endl;

	//	create response object
	auto response = HTTP::Response(202, {
		{"contenty-type", "text/txt"}
	}, "sample response here");
	auto respdump = response.dump();
	std::cout << std::string(respdump.begin(), respdump.end()) << std::endl;
	std::cout << std::endl;

	//	test mimetypes
	std::cout << HTTP::getMimetypeExt("image/jpeg") << std::endl;
	std::cout << HTTP::getExtMimetype("json") << std::endl;
	std::cout << std::endl;

	return 0;
}
