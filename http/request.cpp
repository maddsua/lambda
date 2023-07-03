#include "../lambda.hpp"
#include "./http.hpp"
#include <algorithm>
#include <array>
#include <set>

using namespace Lambda::HTTP;

static const std::set<std::string> httpKnownMethods = {
	"GET",
	"POST",
	"PUT",
	"DELETE",
	"HEAD",
	"OPTIONS",
	"TRACE",
	"PATCH",
	"CONNECT"
};

Request::Request(std::vector<uint8_t>& httpHeadStream) {

	static const std::string patternEndline = "\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHeadStream.begin(), httpHeadStream.end(), patternEndline.begin(), patternEndline.end());

		auto headerLineItems = stringSplit(std::string(httpHeadStream.begin(), httpHeaderLineEnd), " ");

		this->method = stringToUpperCase(stringTrim(static_cast<const std::string>(headerLineItems.at(0))));
		if (httpKnownMethods.find(this->method) == httpKnownMethods.end()) throw std::runtime_error("Unknown http method");

		auto requestURL = URL(headerLineItems.at(1));
		this->path = requestURL.pathname;
		this->searchParams = requestURL.searchParams;

		this->headers.fromHTTP(std::string(httpHeaderLineEnd + patternEndline.size(), httpHeadStream.end()));
		
	} catch(const std::exception& e) {
		throw Lambda::Error(std::string("Request parsing failed: ") + e.what());
	}
}

std::string Request::text() {
	return std::string(this->body.begin(), this->body.end());
}
