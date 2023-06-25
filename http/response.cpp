#include "http.hpp"

void HTTP::Response::setStatusCode(const uint16_t code) {
	auto status = statusText(statusCode);
	if (!status.size()) return;
	this->statusCode = statusCode;
	this->status = status;
}

HTTP::Response::Response() {
	//	do nothing again
}
HTTP::Response::Response(const std::vector<KVtype>& headers) {
	this->headers = headers;
}
HTTP::Response::Response(const uint16_t statusCode) {
	setStatusCode(statusCode);
}
HTTP::Response::Response(const std::vector<uint8_t>& body) {
	this->body = body;
}
HTTP::Response::Response(const std::string& body) {
	this->body = std::vector<uint8_t>(body.begin(), body.end());
}
HTTP::Response::Response(const std::vector<KVtype>& headers, const std::string& body) {
	this->headers = headers;
	this->body = std::vector<uint8_t>(body.begin(), body.end());
}
HTTP::Response::Response(const std::vector<KVtype>& headers, const std::vector<uint8_t>& body) {
	this->headers = headers;
	this->body = body;
}
HTTP::Response::Response(const uint16_t statusCode, const Headers headers) {
	setStatusCode(statusCode);
	this->headers = headers;
}
HTTP::Response::Response(const uint16_t statusCode, const std::string& body) {
	setStatusCode(statusCode);
	this->body = std::vector<uint8_t>(body.begin(), body.end());
}
HTTP::Response::Response(const uint16_t statusCode, const std::vector<uint8_t>& body) {
	setStatusCode(statusCode);
	this->body = body;
}
HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::vector<uint8_t>& body) {
	setStatusCode(statusCode);
	this->headers = headers;
	this->body = body;
}
HTTP::Response::Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body) {
	setStatusCode(statusCode);
	this->headers = headers;
	this->body = std::vector<uint8_t>(body.begin(), body.end());
}

std::vector<uint8_t> HTTP::Response::dump() {
	
	auto frame = "HTTP/1.1 " + std::to_string(this->statusCode) + " " + this->status + "\r\n";
	if (this->body.size()) this->headers.append("content-size", std::to_string(this->body.size()));
	frame += this->headers.stringify();
	frame += "\r\n";
	frame += std::string(this->body.begin(), this->body.end());

	return std::vector<uint8_t>(frame.begin(), frame.end());
}
