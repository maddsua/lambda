#include "http.hpp"
#include <cstdio>

void HTTP::Response::setStatusCode(const uint16_t code) {
	auto status = statusText(code);
	if (!status.size()) return;
	this->_statusCode = code;
	this->_status = status;
}

uint16_t HTTP::Response::statusCode() {
	return this->_statusCode;
}

HTTP::Response::Response() {
	//	do nothing again
}
HTTP::Response::Response(const uint16_t statusCode) {
	setStatusCode(statusCode);
}
HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
}
HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body) {
	setStatusCode(statusCode);
	this->headers.fromEntries(headers);
	this->body = std::vector<uint8_t>(body.begin(), body.end());
}

HTTP::Response& HTTP::Response::operator = (const HTTP::Request& right) {
	auto request = right;
	this->body = request.body();
	this->headers.fromEntries(request.headers().entries());
	return *this;
}

std::vector<uint8_t> HTTP::Response::dump() {

	auto frame = "HTTP/1.1 " + std::to_string(this->_statusCode) + " " + this->_status + "\r\n";
	if (this->body.size()) this->headers.append("content-size", std::to_string(this->body.size()));
	frame += this->headers.stringify();
	frame += "\r\n";
	frame += std::string(this->body.begin(), this->body.end());

	return std::vector<uint8_t>(frame.begin(), frame.end());
}
