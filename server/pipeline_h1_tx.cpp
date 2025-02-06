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


size_t Impl::write_head(Net::TcpConnection& conn, Status status, const Headers& headers) {

	static const char http_version_prefix[] = "HTTP/1.1 ";
	static const char line_break[] = "\r\n";
	static const char header_kv_token[] = ": ";

	auto status_entry = table_status.find(static_cast<Status>(status));
	if (status_entry == table_status.end()) {
		throw std::logic_error("Writer: Unexpected status code");
	}

	auto write_response_line = [&](Status status) -> size_t {

		auto status_code = std::to_string(static_cast<std::underlying_type_t<Status>>(status));

		HTTP::Buffer buff;
		buff.insert(buff.end(), http_version_prefix, static_end(http_version_prefix));
		buff.insert(buff.end(), status_code.begin(), status_code.end());
		buff.push_back(' ');
		buff.insert(buff.end(), status_entry->second.begin(), status_entry->second.end());
		buff.insert(buff.end(), line_break, static_end(line_break));

		return conn.write(HTTP::Buffer(buff.begin(), buff.end()));	
	};

	auto write_header = [&](const std::string& name, const std::string& value) {
		
		HTTP::Buffer buff;
		buff.insert(buff.end(), name.begin(), name.end());
		buff.insert(buff.end(), header_kv_token, static_end(header_kv_token));
		buff.insert(buff.end(), value.begin(), value.end());
		buff.insert(buff.end(), line_break, static_end(line_break));

		return conn.write(HTTP::Buffer(buff.begin(), buff.end()));
	};

	auto bytes_written = write_response_line(status);

	for (const auto& entry : headers.entries()) {
		bytes_written += write_header(entry.first, entry.second);
	}

	bytes_written += conn.write(HTTP::Buffer(line_break, static_end(line_break)));

	return bytes_written;
}
