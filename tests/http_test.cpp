#include <iostream>

#include "../include/lambda/httpcore.hpp"

int main(void) {

	lambda::httpHeaders headers;

	headers.parse("X-maddsua-test: test\r\ncontent-Type: application/json");

	headers.add("X-powered-by", "maddsua/lambda");

	std::cout << headers.find("content-type") << std::endl;

	lambda::httpSearchQuery query;

	query.parse("?test=maddsua&pc=amd");

	std::cout << query.dump() << std::endl;

	auto jsstring = new lambda::jstring("hello");

	jsstring->sstring = "test string data";

	std::cout << jsstring->includes("test") << std::endl;

	return 0;
}