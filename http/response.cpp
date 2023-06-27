#include "./http.hpp"
#include <cstdio>

Lambda::HTTP::Response::Response() {
	//	do nothing again
}
Lambda::HTTP::Response::Response(const uint16_t statusCode) {
	setStatusCode(statusCode);
}
Lambda::HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
}
Lambda::HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
	setBodyText(body);
}
Lambda::HTTP::Response::Response(const std::vector<KVtype>& headers, const std::string& body) {
	this->headers.fromEntries(headers);
	setBodyText(body);
}

Lambda::HTTP::Response& Lambda::HTTP::Response::operator = (const Lambda::HTTP::Request& right) {
	auto request = right;
	this->body = request.body();
	this->headers = request.headers();
	return *this;
}

std::vector<uint8_t> Lambda::HTTP::Response::dump() {

	auto frame = "HTTP/1.1 " + std::to_string(this->_statusCode) + " " + this->_status + "\r\n";
	if (this->body.size()) this->headers.append("content-size", std::to_string(this->body.size()));
	frame += this->headers.stringify();
	frame += "\r\n";
	frame += std::string(this->body.begin(), this->body.end());

	return std::vector<uint8_t>(frame.begin(), frame.end());
}

void Lambda::HTTP::Response::setStatusCode(const uint16_t code) {
	auto status = statusText(code);
	if (!status.size()) return;
	this->_statusCode = code;
	this->_status = status;
}

uint16_t Lambda::HTTP::Response::statusCode() {
	return this->_statusCode;
}

void Lambda::HTTP::Response::setBodyText(const std::string& text) {
	this->body = std::vector<uint8_t>(text.begin(), text.end());
}
