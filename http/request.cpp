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

Request::Request(std::vector<uint8_t>& httpHead) {

	static const std::string patternEndline = "\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHead.begin(), httpHead.end(), patternEndline.begin(), patternEndline.end());

		auto headerLineItems = stringSplit(std::string(httpHead.begin(), httpHeaderLineEnd), " ");

		this->method = stringToUpperCase(stringTrim(static_cast<const std::string>(headerLineItems.at(0))));
		if (httpKnownMethods.find(this->method) == httpKnownMethods.end()) throw std::runtime_error("Unknown http method");

		const auto headerlinePath = stringToLowerCase(stringTrim(static_cast<const std::string>(headerLineItems.at(1))));
		auto pathSearchQueryIdx = headerlinePath.find_first_of('?');
		
		if (pathSearchQueryIdx != std::string::npos) {
			this->path = headerlinePath.substr(0, headerlinePath.find_first_of('?'));
			this->searchParams.fromHref(headerlinePath.substr(headerlinePath.find_first_of('?') + 1));
		} else this->path = headerlinePath;

		this->headers.fromHTTP(std::string(httpHeaderLineEnd + patternEndline.size(), httpHead.end()));
		
	} catch(const std::exception& e) {
		throw Lambda::Error(std::string("Request parsing failed: ") + e.what());
	}
}

std::string Request::text() {
	return std::string(this->body.begin(), this->body.end());
}
