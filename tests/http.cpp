#include "../http/http.hpp"
#include <cstdio>
#include <exception>
#include <stdexcept>

using namespace Lambda;

int main() {

	puts("Search query generation test...");
	auto searchQuery = HTTP::URLSearchParams();

	searchQuery.set("id", "12345");
	searchQuery.set("hl", "en");
	searchQuery.set("state", "de");

	if (searchQuery.stringify() != "hl=en&id=12345&state=de")
		throw std::runtime_error("Search query generation failed, result: " + searchQuery.stringify());
	puts("Ok\n");


	puts("URL generation test...");
	auto urladdress = HTTP::URL();

	urladdress.setHref("http://localhost/test");
	urladdress.searchParams = searchQuery;

	if (urladdress.href() != "http://localhost/test?hl=en&id=12345&state=de")
		throw std::runtime_error("URL generation failed, result: " + urladdress.href());
	puts("Ok\n");


	puts("Request parsing test...");
	auto reqtext = std::string("GET /test HTTP/1.1\r\nTest-header    :    lambda\r\ncontent-size: 14\r\n\r\n");
	auto req = HTTP::Request(std::vector<uint8_t>(reqtext.begin(), reqtext.end()));

	if (req.headers.get("test-header") != "lambda")
		throw std::runtime_error("failed to get 'test-header', got: " + req.headers.get("test-header"));

	if (req.headers.get("content-size") != "14")
		throw std::runtime_error("failed to get 'content-size', got: " + req.headers.get("content-size"));

	if (req.method != "GET")
		throw std::runtime_error("failed to get request method, got: " + req.method);

	if (req.url.pathname != "/test")
		throw std::runtime_error("failed to get request url, got: " + req.url.pathname);

	puts("Ok\n");


	puts("Response composition test...");
	auto response = HTTP::Response(202, {
		{"contenty-type", "text/plain"}
	}, "sample response here");
	auto respdump = response.dump();
	auto dumpTextVerify = "HTTP/1.1 202 Accepted\r\nContenty-Type: text/plain\r\nContent-Size: 20\r\n\r\nsample response here";
	auto dumpText = std::string(respdump.begin(), respdump.end());

	if (dumpText != dumpTextVerify)
		throw std::runtime_error("HTTP text mismatch, got: " + dumpText);

	puts("Ok\n");


	puts("MimeType matching test...");

	if (HTTP::getMimetypeExt("image/jpeg") != "jpeg")
		throw std::runtime_error("Incorrect mimetype for 'image/jpeg', got: " + HTTP::getMimetypeExt("image/jpeg"));

	if (HTTP::getMimetypeExt("application/json") != "json")
		throw std::runtime_error("Incorrect mimetype for 'application/json', got: " + HTTP::getMimetypeExt("application/json"));

	puts("Ok\n");


	return 0;
}
