#include "./http.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <string.h>

using namespace Lambda::HTTP;

Request::Request(std::vector<uint8_t>& httpHead) {

	static const std::string patternEndline = "\r\n";
	static const std::string patternEndHeader = "\r\n\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHead.begin(), httpHead.end(), patternEndline.begin(), patternEndline.end());
		auto httpHeaderEnd = std::search(httpHead.begin(), httpHead.end(), patternEndHeader.begin(), patternEndHeader.end());
		auto headerLineItems = stringSplit(std::string(httpHead.begin(), httpHeaderLineEnd), " ");

		this->method = stringToUpperCase(stringTrim(static_cast<const std::string>(headerLineItems.at(0))));

		const auto headerlinePath = stringToLowerCase(stringTrim(static_cast<const std::string>(headerLineItems.at(1))));
		auto pathSearchQueryIdx = headerlinePath.find_first_of('?');
		
		if (pathSearchQueryIdx != std::string::npos) {
			this->path = headerlinePath.substr(0, headerlinePath.find_first_of('?'));
			this->searchParams = URLSearchParams(headerlinePath.substr(headerlinePath.find_first_of('?') + 1));
		} else this->path = headerlinePath;

		if (httpHeaderLineEnd != httpHeaderEnd)
			this->headers.fromHTTP(std::string(httpHeaderLineEnd + patternEndline.size(), httpHeaderEnd));
		
	} catch(const std::exception& e) {
		//std::cerr << e.what() << '\n';
		//puts("ooooops!");
		//	haha not doint anything lol
	}

}

std::string Request::text() {
	return std::string(this->body.begin(), this->body.end());
}
