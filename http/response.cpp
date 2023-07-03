#include "../lambda.hpp"
#include "./http.hpp"
#include <algorithm>

using namespace Lambda;
using namespace Lambda::HTTP;

Response::Response(const std::vector<uint8_t>& httpHeadStream) {

	static const std::string patternEndline = "\r\n";

	try {

		auto httpHeaderLineEnd = std::search(httpHeadStream.begin(), httpHeadStream.end(), patternEndline.begin(), patternEndline.end());

		auto headerLineItems = stringSplit(std::string(httpHeadStream.begin(), httpHeaderLineEnd), " ");

		this->setStatusCode(std::stoi(headerLineItems.at(1)));

		this->headers.fromHTTP(std::string(httpHeaderLineEnd + patternEndline.size(), httpHeadStream.end()));
		
	} catch(const std::exception& e) {
		throw Lambda::Error(std::string("Request parsing failed: ") + e.what());
	}
}

Response::Response(const uint16_t statusCode) {
	setStatusCode(statusCode);
}
Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
}
Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
	setBodyText(body);
}
Response::Response(const std::vector<KVtype>& headers, const std::string& body) {
	this->headers.fromEntries(headers);
	setBodyText(body);
}

Response& Response::operator = (const Request& right) {
	auto request = right;
	this->body = request.body;
	this->headers = request.headers;
	return *this;
}

std::vector<uint8_t> Response::dump() {

	auto frame = "HTTP/1.1 " + std::to_string(this->_statusCode) + " " + this->_status + "\r\n";
	if (this->body.size()) this->headers.append("content-size", std::to_string(this->body.size()));
	frame += this->headers.stringify();
	frame += "\r\n";
	frame += std::string(this->body.begin(), this->body.end());

	return std::vector<uint8_t>(frame.begin(), frame.end());
}

void Response::setStatusCode(const uint16_t code) {
	auto status = statusText(code);
	if (!status.size()) return;
	this->_statusCode = code;
	this->_status = status;
}

uint16_t Response::statusCode() {
	return this->_statusCode;
}

void Response::setBodyText(const std::string& text) {
	this->body = std::vector<uint8_t>(text.begin(), text.end());
}
