#include "./pipelines.hpp"
#include "../version.hpp"

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

size_t Impl::ResponseWriter::flush() {

	if (!this->m_conn.is_open()) {
		return 0;
	}

	if (!this->m_header_written) {
		this->m_headers.set("content-length", std::to_string(0));
		return Impl::write_head(this->m_conn, Status::OK, this->m_headers);
	}

	if (this->m_deferred.has_value()) {

		auto& deferred = this->m_deferred.value();
		
		//	set content length to terminate response
		deferred.headers.set("content-length", std::to_string(deferred.body.size()));

		auto bytes_written = Impl::write_head(this->m_conn, deferred.status, deferred.headers);			
		bytes_written += this->m_conn.write(deferred.body);

		return bytes_written;
	}

	return 0;
}

bool Impl::ResponseWriter::writable() const noexcept {
	return this->m_conn.is_open();
}

Headers& Impl::ResponseWriter::header() noexcept {
	return this->m_headers;
}

size_t Impl::ResponseWriter::write_header() {
	return this->write_header(Status::OK);
}

size_t Impl::ResponseWriter::write_header(Status status) {

	if (this->m_header_written) {
		throw std::runtime_error("Response header has been already written");
	}

	this->m_header_written = true;

	//	flag upgraded connections (eg to websockets)
	if (HTTP::reset_case(this->m_headers.get("connection")).contains("upgrade")) {
		
		this->m_headers.del("content-length");

		this->m_stream.http_keep_alive = false;
		this->m_stream.http_body_pending = 0;
		this->m_stream.raw_io = true;

		//	set raw read timeout to a small value
		this->m_conn.set_timeouts({ .read = Impl::raw_io_read_timeout, .write = this->m_conn.timeouts().write });
	}

	//	flag streaming responses such as sse
	if (HTTP::reset_case(this->m_headers.get("content-type")).contains("event-stream")) {
		this->m_headers.del("content-length");
		this->m_stream.http_keep_alive = false;
		this->m_stream.stream_response = true;
	}

	//	set some utility headers
	if (!this->m_headers.has("connection")) {
		this->m_headers.set("connection", this->m_stream.http_keep_alive ? "keep-alive" : "close");
	}

	if (!this->m_headers.has("date")) {
		this->m_headers.set("date", Date().to_utc_string());
	}

	if (!this->m_headers.has("server")) {
		this->m_headers.set("server", LAMBDA_SERVER_HEADER);
	}

	if (!this->m_headers.has("cache-control")) {
		this->m_headers.set("cache-control", "no-cache");
	}

	//	defer response if no content length is provided
	auto body_deferrable = !(this->m_stream.stream_response || this->m_stream.raw_io);
	auto content_length = this->m_headers.get("content-length");
	if (body_deferrable && content_length.empty()) {

		this->m_deferred = Impl::DeferredResponse {
			.headers = std::move(this->m_headers),
			.status = status,
		};

		this->m_headers = {};

		return 0;

	} else if (!content_length.empty()) {
		this->m_announced = std::stoul(content_length);
	}

	return Impl::write_head(this->m_conn, status, this->m_headers);
}

size_t Impl::ResponseWriter::write(const HTTP::Buffer& data) {

	if (!this->m_header_written) {
		this->write_header(Status::OK);
	}

	if (this->m_deferred.has_value()) {
		auto& deferred = this->m_deferred.value();
		deferred.body.insert(deferred.body.end(), data.begin(), data.end());
		return data.size();
	}

	if (this->m_announced) {

		//	ensure we aren't writing over the specificed content-length
		auto announced = this->m_announced.value();
		if (this->m_body_written >= announced) {
			return 0;
		}

		if (this->m_ctx.opts.debug) {
			fprintf(stderr, "%s DEBUG Lambda::Serve::H1 { remote_addr='%s', conn=%i }: Response truncated (%lu/%lu)\n",
				Date().to_log_string().c_str(),
				this->m_conn.remote_addr().hostname.c_str(),
				this->m_conn.id(),
				announced,
				this->m_body_written + data.size()
			);
		}

		//	write truncated response
		auto can_write = announced - this->m_body_written;
		if (data.size() > can_write) {
			auto chunk = HTTP::Buffer(data.begin(), data.begin() + can_write);
			auto bytes_written = this->m_conn.write(chunk);
			this->m_body_written += bytes_written;
			return bytes_written;
		}
	}

	auto bytes_written = this->m_conn.write(data);
	this->m_body_written += bytes_written;
	return bytes_written;
}

size_t Impl::ResponseWriter::write(const std::string& text) {
	return this->write(HTTP::Buffer(text.begin(), text.end()));
}

void Impl::ResponseWriter::set_cookie(const Cookie& cookie) {
	this->header().append("Set-Cookie", cookie.to_string());
}

bool Impl::ResponseWriter::is_deferred() const noexcept {
	return this->m_deferred.has_value();
}
