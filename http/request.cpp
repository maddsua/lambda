#include "./http.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <string.h>

Lambda::HTTP::Request::Request(std::vector<uint8_t>& httpHead) {

	static const std::string patternEndline = "\r\n";
	static const std::string patternEndHeader = "\r\n\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHead.begin(), httpHead.end(), patternEndline.begin(), patternEndline.end());
		auto httpHeaderEnd = std::search(httpHead.begin(), httpHead.end(), patternEndHeader.begin(), patternEndHeader.end());
		auto headerLineItems = stringSplit(std::string(httpHead.begin(), httpHeaderLineEnd), " ");

		const auto method = headerLineItems.at(0);
		this->_method = stringToUpperCase(method);

		const auto path = headerLineItems.at(1);
		auto pathSearchQueryIdx = path.find_first_of('?');
		if (pathSearchQueryIdx != std::string::npos) {
			this->_path = path.substr(0, path.find_first_of('?'));
			this->_searchParams = URLSearchParams(path.substr(path.find_first_of('?') + 1));
		} else this->_path = path;

		if (httpHeaderLineEnd != httpHeaderEnd)
			this->_headers = Headers(std::string(httpHeaderLineEnd + patternEndline.size(), httpHeaderEnd));
		

	} catch(const std::exception& e) {
		//std::cerr << e.what() << '\n';
		//puts("ooooops!");
		//	haha not doint anything lol
	}

}

void Lambda::HTTP::Request::_setBody(std::vector<uint8_t>& content) {
	this->_body = content;
}
void Lambda::HTTP::Request::_appendBody(std::vector<uint8_t>& content) {
	this->_body.insert(this->_body.end(), content.begin(), content.end());
}

std::string Lambda::HTTP::Request::method() {
	return this->_method;
}
std::string Lambda::HTTP::Request::path() {
	return this->_path;
}
Lambda::HTTP::URLSearchParams Lambda::HTTP::Request::searchParams() {
	return this->_searchParams;
}

Lambda::HTTP::Headers Lambda::HTTP::Request::headers() {
	return this->_headers;
}

std::vector<uint8_t> Lambda::HTTP::Request::body() {
	return this->_body;
}
std::string Lambda::HTTP::Request::text() {
	return std::string(this->_body.begin(), this->_body.end());
}
