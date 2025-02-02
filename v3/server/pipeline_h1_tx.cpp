#include "./pipelines.hpp"

#include <cstdint>
#include <map>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

#define static_end(string) (string + (sizeof(string) - 1))

static const std::map<Status, std::string> table_status = {
	{ Status::Continue, "Continue" },
	{ Status::SwitchingProtocols, "Switching Protocols" },
	{ Status::Processing, "Processing" },

	{ Status::OK, "OK" },
	{ Status::Created, "Created" },
	{ Status::Accepted, "Accepted" },
	{ Status::NonAuthoritativeInfo, "Non-Authoritative Information" },
	{ Status::NoContent, "No Content" },
	{ Status::ResetContent, "Reset Content" },
	{ Status::PartialContent, "Partial Content" },
	{ Status::MultiStatus, "Multi-Status" },
	{ Status::IMUsed, "IM Used" },

	{ Status::MultipleChoices, "Multiple Choices" },
	{ Status::MovedPermanently, "Moved Permanently" },
	{ Status::Found, "Found" },
	{ Status::SeeOther, "See Other" },
	{ Status::NotModified, "Not Modified" },
	{ Status::UseProxy, "Use Proxy" },
	{ Status::TemporaryRedirect, "Temporary Redirect" },
	{ Status::PermanentRedirect, "Permanent Redirect" },

	{ Status::BadRequest, "Bad Request" },
	{ Status::Unauthorized, "Unauthorized" },
	{ Status::PaymentRequired, "Payment Required" },
	{ Status::Forbidden, "Forbidden" },
	{ Status::NotFound, "Not Found" },
	{ Status::MethodNotAllowed, "Method Not Allowed" },
	{ Status::NotAcceptable, "Not Acceptable" },
	{ Status::ProxyAuthRequired, "Proxy Authentication Required" },
	{ Status::RequestTimeout, "Request Timeout" },
	{ Status::Conflict, "Conflict" },
	{ Status::Gone, "Gone" },
	{ Status::LengthRequired, "Length Required" },
	{ Status::PreconditionFailed, "Precondition Failed" },
	{ Status::RequestEntityTooLarge, "Payload Too Large" },
	{ Status::RequestURITooLong, "URI Too Long" },
	{ Status::UnsupportedMediaType, "Unsupported Media Type" },
	{ Status::RequestedRangeNotSatisfiable, "Range Not Satisfiable" },
	{ Status::ExpectationFailed, "Expectation Failed" },
	{ Status::Teapot, "I'm a teapot" },
	{ Status::UnprocessableEntity, "Unprocessable Entity" },
	{ Status::Locked, "Locked" },
	{ Status::FailedDependency, "Failed Dependency" },
	{ Status::UpgradeRequired, "Upgrade Required" },
	{ Status::PreconditionRequired, "Precondition Required" },
	{ Status::TooManyRequests, "Too Many Requests" },
	{ Status::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
	{ Status::UnavailableForLegalReasons, "Unavailable For Legal Reasons" },

	{ Status::InternalServerError, "Internal Server Error" },
	{ Status::NotImplemented, "Not Implemented" },
	{ Status::BadGateway, "Bad Gateway" },
	{ Status::ServiceUnavailable, "Service Unavailable" },
	{ Status::GatewayTimeout, "Gateway Time-out" },
	{ Status::HTTPVersionNotSupported, "HTTP Version Not Supported" },
	{ Status::VariantAlsoNegotiates, "Variant Also Negotiates" },
	{ Status::InsufficientStorage, "Insufficient Storage" },
	{ Status::NetworkAuthenticationRequired, "Network Authentication Required" }
};


size_t Impl::write_head(Net::TcpConnection& conn, int status, const Headers& headers) {

	static const char http_version_prefix[] = "HTTP/1.1 ";
	static const char line_break[] = "\r\n";
	static const char header_kv_token[] = ": ";

	auto status_entry = table_status.find(static_cast<Status>(status));
	if (status_entry == table_status.end()) {
		throw std::logic_error("Writer: Unexpected status code");
	}

	auto status_code = std::to_string(static_cast<std::underlying_type_t<Status>>(status));

	HTTP::Buffer response_line;
	response_line.insert(response_line.end(), http_version_prefix, static_end(http_version_prefix));
	response_line.insert(response_line.end(), status_code.begin(), status_code.end());
	response_line.push_back(' ');
	response_line.insert(response_line.end(), status_entry->second.begin(), status_entry->second.end());
	response_line.insert(response_line.end(), line_break, static_end(line_break));

	auto bytes_written = conn.write(HTTP::Buffer(response_line.begin(), response_line.end()));

	for (const auto& entry : headers.entries()) {
		
		HTTP::Buffer next;
		next.insert(next.end(), entry.first.begin(), entry.first.end());
		next.insert(next.end(), header_kv_token, static_end(header_kv_token));
		next.insert(next.end(), entry.second.begin(), entry.second.end());
		next.insert(next.end(), line_break, static_end(line_break));

		bytes_written += conn.write(HTTP::Buffer(next.begin(), next.end()));
	}
	
	bytes_written += conn.write(HTTP::Buffer(line_break, static_end(line_break)));

	return bytes_written;
}

void Impl::write_request_error(Net::TcpConnection& conn, Status status, std::string message) {

	Headers response_headers;
	
	response_headers.set("connection", "close");
	response_headers.set("content-type", "text/plain");
	response_headers.set("content-length", std::to_string(message.size()));
	Impl::set_response_meta(response_headers);

	Impl::write_head(conn, static_cast<std::underlying_type_t<Status>>(status), response_headers);
	conn.write(HTTP::Buffer(message.begin(), message.end()));
}

void Impl::set_response_meta(Headers& headers) {

	if (!headers.has("date")) {
		headers.set("date", Date().to_utc_string());
	}

	if (!headers.has("server")) {
		headers.set("server", "maddsua/l3");
	}
}
