#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

#include <stdexcept>
#include <map>

using namespace Lambda::HTTP;
using namespace Lambda::Strings;

static const std::map <int, std::string> statusCodeTable = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 102, "Processing" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 207, "Multi-Status" },
	{ 226, "IM Used" },

	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },

	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Payload Too Large" },
	{ 414, "URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Range Not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 418, "I'm a teapot" },
	{ 422, "Unprocessable Entity" },
	{ 423, "Locked" },
	{ 424, "Failed Dependency" },
	{ 426, "Upgrade Required" },
	{ 428, "Precondition Required" },
	{ 429, "Too Many Requests" },
	{ 431, "Request Header Fields Too Large" },
	{ 451, "Unavailable For Legal Reasons" },
	
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Time-out" },
	{ 505, "HTTP Version Not Supported" },
	{ 506, "Variant Also Negotiates" },
	{ 507, "Insufficient Storage" },
	{ 511, "Network Authentication Required" }
};

Status::Status() {
	this->m_code = 200;
	this->m_text = "OK";
}

Status::Status(uint32_t code, const std::string& text) {
	this->m_code = code;
	this->m_text = text;
}

Status::Status(uint32_t code) {

	const auto foundCode = statusCodeTable.find(code);
	if (foundCode == statusCodeTable.end())
		throw std::invalid_argument("provided http status code is unknown");

	this->m_code = foundCode->first;
	this->m_text = foundCode->second;
}

uint32_t Status::code() const noexcept {
	return this->m_code;
}

const std::string& Status::text() const noexcept {
	return this->m_text;
}

Status::Type Status::type() const noexcept {

	if (this->m_code < 200) return Type::Info;
	else if (this->m_code >= 200 && this->m_code < 300) return Type::Success;
	else if (this->m_code >= 300 && this->m_code < 400) return Type::Redirect;
	else if (this->m_code >= 400 && this->m_code < 500) return Type::ClientError;
	else if (this->m_code >= 500) return Type::ServerError;
	
	return Type::Unknown;
}
