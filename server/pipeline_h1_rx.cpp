#include "./pipelines.hpp"

#include <cstdint>
#include <map>
#include <expected>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

size_t next_space(HTTP::Buffer& data, size_t begin, size_t end) {

	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == ' ') {
			return idx;
		}
	}

	return Impl::nullidx;
}

size_t next_token(HTTP::Buffer& data, size_t begin, size_t end) {

	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] != ' ') {
			return idx;
		}
	}

	return Impl::nullidx;
}

size_t next_line(HTTP::Buffer& data, size_t begin, size_t end) {

	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == '\n' && idx > begin && data[idx - 1] == '\r') {
			return idx - 1;
		}
	}

	return Impl::nullidx;
}

URL parse_request_url(HTTP::Buffer& data, size_t begin, size_t end) {

	URL url;

	//	find page anchor, if exists (it really should not)
	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == '#') {
			url.fragment = std::string(data.begin() + idx, data.begin() + end);
			end = idx;
			break;
		}
	}
	
	//	find query params
	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == '?') {
			url.search = URLSearchParams(std::string(data.begin() + idx + 1, data.begin() + end));
			begin = end;
			break;
		}
	}

	//	lastly, get the path
	url.path = std::string(data.begin() + begin, data.begin() + end);

	return url;
}

bool is_valid_http_version(HTTP::Buffer& data, size_t begin, size_t end) {

	auto prefix_end = std::string::npos;
	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == '/') {
			prefix_end = idx;
			break;
		}
	}

	if (prefix_end == std::string::npos) {
		return false;
	}

	auto version_start = prefix_end + 1;
	if (version_start >= end) {
		return false;
	}

	auto prefix = HTTP::reset_case(std::string(data.begin() + begin, data.begin() + prefix_end));
	auto version = std::string(data.begin() + version_start, data.begin() + end);

	return prefix == "http" && (version == "1.0" || version == "1.1");
}

std::expected<Impl::RequestHead, std::string> parse_request_line(
	Impl::RequestHead& req,
	HTTP::Buffer& data,
	size_t begin,
	size_t end
) {

	// skip possible whitespaces
	begin = next_token(data, begin, end);
	if (begin == Impl::nullidx) {
		return std::unexpected("Request line invalid: invalid spacing");
	}

	//	get request method
	auto tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		return std::unexpected("Request line invalid: no defined method");
	}

	auto method_opt = HTTP::string_to_method(std::string(data.begin() + begin, data.begin() + tokenEnd));
	if (!method_opt.has_value()) {
		return std::unexpected("Invlid http method");
	}

	//	skip to url
	begin = next_token(data, tokenEnd, end);
	if (begin == Impl::nullidx) {
		return std::unexpected("Request line invalid: invalid spacing");
	}

	//	get request url
	tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		return std::unexpected("Request line invalid: no defined url");
	}

	auto url = parse_request_url(data, begin, tokenEnd);

	//	skip to version
	begin = next_token(data, tokenEnd, end);
	if (begin == Impl::nullidx) {
		return std::unexpected("Request line invalid: invalid spacing");
	}

	//	get http version
	tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		tokenEnd = end;
	}

	if (!is_valid_http_version(data, begin, tokenEnd)) {
		return std::unexpected("Unsupported http version");
	}

	return Impl::RequestHead {
		.method = method_opt.value(),
		.url = url
	};
}

void parse_header(Headers& headers, HTTP::Buffer& data, size_t begin, size_t end) {

	size_t split = Impl::nullidx;

	begin = next_token(data, begin, end);
	if (begin == Impl::nullidx) {
		return;
	}

	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == ' ' && idx > begin && data[idx - 1] == ':') {
			split = idx - 1;
			break;
		}
	}
	
	if (split ==Impl:: nullidx || split - begin < 1) {
		return;
	}

	auto key = std::string(data.begin() + begin, data.begin() + split);

	begin = next_token(data, split + 2, end);
	if (begin == Impl::nullidx || end - begin < 1) {
		return;
	}

	headers.append(key, std::string(data.begin() + begin, data.begin() + end));
}

void merge_url_components(URL& url, const Headers& headers) {

	url.scheme = "http";
	url.user = HTTP::parse_basic_auth(headers.get("authorization"));

	url.host = headers.get("host");
	if (url.host.empty()) {
		url.host = "localhost";
	}

	if (url.path.empty()) {
		url.path = "/";
	}
}

std::expected<Impl::RequestHead, Impl::RequestError> Impl::read_request_head(
	Net::TcpConnection& conn,
	HTTP::Buffer& read_buff,
	const ServeContext& ctx
) {

	size_t reader_seek = 0;
	size_t chunker_seek = nullidx;
	size_t total_read = 0;

	RequestHead next;
	bool request_line_parsed = false;

	while (!ctx.done()) {

		auto seek_begin = chunker_seek != nullidx ? chunker_seek : reader_seek;

		auto line_end = next_line(read_buff, seek_begin, read_buff.size());
		if (line_end == nullidx) {

			if (!conn.is_open()) {
				return std::unexpected<Impl::RequestError>({
					"connection closed",
					Status::Teapot,
				});
			}

			if (total_read > ctx.opts.max_header_size) {
				return std::unexpected<Impl::RequestError>({
					"request header section is too big",
					Status::RequestHeaderFieldsTooLarge,
				});
			}

			auto next_chunk = conn.read(header_chunk_size);
			if (next_chunk.empty()) {
				return std::unexpected<Impl::RequestError>({
					"request timed out or client disconnected",
					Status::RequestTimeout,
					true,
				});
			}

			total_read += next_chunk.size();
			chunker_seek = read_buff.size() - 1;
			read_buff.insert(read_buff.end(), next_chunk.begin(), next_chunk.end());

			continue;
		}

		auto line_begin = reader_seek;
		reader_seek = line_end + 2;
		chunker_seek = nullidx;

		if (!request_line_parsed) {

			auto req_line = parse_request_line(next, read_buff, line_begin, line_end);
			if (!req_line.has_value()) {
				return std::unexpected<Impl::RequestError>({
					req_line.error(),
					Status::BadRequest,
				});
			}

			next = req_line.value();
			request_line_parsed = true;
		}

		if (line_end - line_begin <= 2) {
			read_buff.erase(read_buff.begin(), read_buff.begin() + line_end + 2);
			break;
		}

		parse_header(next.headers, read_buff, line_begin, line_end);
	}

	merge_url_components(next.url, next.headers);

	return next;
}

std::optional<size_t> Impl::content_length(const Headers& headers) {

	auto content_length = headers.get("content-length");
	if (content_length.empty()) {
		return std::nullopt;
	}

	return std::strtoul(content_length.c_str(), nullptr, 10);
}

HTTP::Buffer Impl::read_request_body(
	Net::TcpConnection& conn,
	HTTP::Buffer& read_buff,
	size_t chunk_size,
	size_t bytes_remain
) {

	auto can_read = bytes_remain > chunk_size ? chunk_size : bytes_remain;
	if (can_read <= 0) {
		return {};
	}

	//	exhaust the read buffer first
	if (!read_buff.empty()) {

		if (can_read >= read_buff.size()) {
			HTTP::Buffer chunk;
			chunk.swap(read_buff);
			read_buff.clear();
			return chunk;
		}

		const auto chunk = HTTP::Buffer(read_buff.begin(), read_buff.begin() + can_read);
		read_buff.erase(read_buff.begin(), read_buff.begin() + can_read);
		return chunk;
	}

	//	read remaining data from the connection
	return conn.read(can_read);
}

HTTP::Buffer Impl::read_raw_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size) {

	if (!read_buff.empty()) {
		HTTP::Buffer chunk;
		chunk.swap(read_buff);
		read_buff.clear();
		return chunk;
	}

	return conn.read(chunk_size);
}

void Impl::discard_unread_body(Net::TcpConnection& conn, StreamState& stream) {

	if (stream.raw_io || stream.http_body_pending <= 0) {
		return;
	}

	if (!stream.read_buff.empty()) {
		auto discard_buff_size = stream.http_body_pending <= stream.read_buff.size()
			? stream.http_body_pending : stream.read_buff.size();
		stream.read_buff.erase(stream.read_buff.begin(), stream.read_buff.begin() + discard_buff_size);
		stream.http_body_pending -= discard_buff_size;
	}

	if (stream.http_body_pending > 0) {
		stream.http_body_pending -= conn.read(stream.http_body_pending).size();
	}
}

bool Impl::RequestBodyReader::is_readable() const noexcept {
	return this->m_conn.is_open();
}

HTTP::Buffer Impl::RequestBodyReader::read(size_t chunk_size) {

	if (!this->is_readable()) {
		return {};
	}

	//	patch chunk size
	if (chunk_size == HTTP::LengthUnknown || chunk_size <= 0) {
		chunk_size = Impl::body_chunk_size;
	}

	//	stream reads (stuff like websockets)
	if (this->m_stream.raw_io) {
		return Impl::read_raw_body(this->m_conn, this->m_stream.read_buff, chunk_size);
	}

	//	return early if body is empty or consoomed
	if (this->m_stream.http_body_pending <= 0) {
		return {};
	}

	//	return body from streams
	auto chunk = Impl::read_request_body(this->m_conn, this->m_stream.read_buff, chunk_size, this->m_stream.http_body_pending);
	this->m_stream.http_body_pending -= chunk.size();

	return chunk;
}

HTTP::Buffer Impl::RequestBodyReader::read_all() {
	return this->read(HTTP::LengthUnknown);
}

std::string Impl::RequestBodyReader::text() {
	auto buffer = this->read_all();
	return std::string(buffer.begin(), buffer.end());
}
