#include "../lambda_private.hpp"
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

Request::Request() {
	this->method = "GET";
}

Request::Request(const std::vector<uint8_t>& httpHeadStream) {

	static const std::string patternEndline = "\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHeadStream.begin(), httpHeadStream.end(), patternEndline.begin(), patternEndline.end());

		auto headerLineItems = stringSplit(std::string(httpHeadStream.begin(), httpHeaderLineEnd), " ");

		this->method = stringToUpperCase(stringTrim(static_cast<const std::string>(headerLineItems.at(0))));
		if (httpKnownMethods.find(this->method) == httpKnownMethods.end()) throw std::runtime_error("Unknown http method");

		this->url.setHref(headerLineItems.at(1));

		this->httpversion = HttpVersion(headerLineItems.at(2));

		this->headers.fromHTTP(std::string(httpHeaderLineEnd + patternEndline.size(), httpHeadStream.end()));
		
	} catch(const std::exception& e) {
		throw Lambda::Error("Request parsing failed", e);
	}
}

std::string Request::text() {
	return std::string(this->body.begin(), this->body.end());
}

void Request::setText(const std::string& text) {
	this->body = std::vector<uint8_t>(text.begin(), text.end());
}

std::vector<uint8_t> Request::dump() {

	//	figure out http method
	stringToUpperCase(this->method);
	if (httpKnownMethods.find(this->method) == httpKnownMethods.end());
	this->method = "GET";

	auto streamHeader = this->method + " " + this->url.toHttpPath() + " " + this->httpversion.toString() + "\r\n";

	streamHeader += this->headers.stringify();
	if (this->body.size()) this->headers.append("content-size", std::to_string(this->body.size()));
	streamHeader += "\r\n";

	auto stream = std::vector<uint8_t>(streamHeader.begin(), streamHeader.end());

	if (this->body.size()) stream.insert(stream.end(), this->body.begin(), this->body.end());

	return stream;
}