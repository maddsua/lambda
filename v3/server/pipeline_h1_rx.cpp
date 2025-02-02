#include "./pipelines.hpp"

#include <cstdint>
#include <map>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

static const std::map<std::string, Method> table_method = {
	{ "GET", Method::GET },
	{ "POST", Method::POST },
	{ "PUT", Method::PUT },
	{ "DELETE", Method::DEL },
	{ "HEAD", Method::HEAD },
	{ "OPTIONS", Method::OPTIONS },
	{ "TRACE", Method::TRACE },
	{ "PATCH", Method::PATCH },
	{ "CONNECT", Method::CONNECT },
};

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


void parse_request_url(URL& url, HTTP::Buffer& data, size_t begin, size_t end) {

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
}

bool is_valid_http_version(HTTP::Buffer& data, size_t begin, size_t end) {
	return true;
}

std::optional<std::runtime_error> parse_request_line(Request& req, HTTP::Buffer& data, size_t begin, size_t end) {

	// skip possible whitespaces
	begin = next_token(data, begin, end);
	if (begin == Impl::nullidx) {
		return std::runtime_error("Request line invalid: invalid spacing");
	}

	//	get request method
	auto tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		return std::runtime_error("Request line invalid: no defined method");
	}

	auto methodEntry = table_method.find(std::string(data.begin() + begin, data.begin() + tokenEnd));
	if (methodEntry == table_method.end()) {
		return std::runtime_error("invalid request method");
	}

	req.method = methodEntry->second;

	//	skip to url
	begin = next_token(data, tokenEnd, end);
	if (begin == Impl::nullidx) {
		return std::runtime_error("Request line invalid: invalid spacing");
	}

	//	get request url
	tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		return std::runtime_error("Request line invalid: no defined url");
	}

	parse_request_url(req.url, data, begin, tokenEnd);

	//	skip to version
	begin = next_token(data, tokenEnd, end);
	if (begin == Impl::nullidx) {
		return std::runtime_error("Request line invalid: invalid spacing");
	}

	//	get http version
	tokenEnd = next_space(data, begin, end);
	if (tokenEnd == Impl::nullidx) {
		tokenEnd = end;
	}

	if (!is_valid_http_version(data, begin, tokenEnd)) {
		return std::runtime_error("Unsupported/invalid http version");
	}

	return std::nullopt;
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

std::expected<Request, Impl::RequestError> Impl::read_request_head(Net::TcpConnection& conn, HTTP::Buffer& read_buff, ServerContext ctx) {

	auto opts = ctx.options();

	size_t reader_seek = 0;
	size_t chunker_seek = nullidx;

	bool request_line_parsed = false;

	Request next {
		.remote_addr = conn.remote_addr(),
	};

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

			if (read_buff.size() > opts.max_header_size) {
				return std::unexpected<Impl::RequestError>({
					"your headers are a bit too big",
					Status::RequestHeaderFieldsTooLarge,
				});
			}

			auto next_chunk = conn.read(header_chunk_size);
			if (!next_chunk.size()) {
				return std::unexpected<Impl::RequestError>({
					"request timed out or client disconnected",
					Status::RequestTimeout,
					true,
				});
			}

			chunker_seek = read_buff.size() - 1;
			read_buff.insert(read_buff.end(), next_chunk.begin(), next_chunk.end());

			continue;
		}

		auto line_begin = reader_seek;
		reader_seek = line_end + 2;
		chunker_seek = nullidx;

		if (!request_line_parsed) {

			auto parse_error = parse_request_line(next, read_buff, line_begin, line_end);
			if (parse_error.has_value()) {
				return std::unexpected<Impl::RequestError>({
					parse_error.value().what(),
					Status::BadRequest,
				});
			}

			request_line_parsed = true;
		}

		if (line_end - line_begin <= 2) {

			read_buff.erase(read_buff.begin(), read_buff.begin() + line_end + 2);

			next.url.host = next.headers.get("host");
			next.url.scheme = "http";
			next.cookies = HTTP::parse_cookie(next.headers.get("cookie"));

			//	todo: parse basic auth header

			break;
		}

		parse_header(next.headers, read_buff, line_begin, line_end);
	}

	return next;
}

bool Impl::method_can_have_body(Method method) {
	return method == Method::POST || method == Method::PUT || method == Method::PATCH || method == Method::CONNECT;
}

std::optional<size_t> Impl::content_length(const Headers& headers) {

	auto content_length = headers.get("content-length");
	if (!content_length.size()) {
		return std::nullopt;
	}

	return std::strtoul(content_length.c_str(), nullptr, 10);
}

HTTP::Buffer Impl::read_request_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size, size_t bytes_remain) {

	auto can_read = bytes_remain > chunk_size ? chunk_size : bytes_remain;
	if (can_read <= 0) {
		return {};
	}

	//	exhaust the read buffer first
	if (read_buff.size()) {

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

	if (read_buff.size()) {
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

	if (stream.read_buff.size()) {
		auto discard_buff_size = stream.http_body_pending <= stream.read_buff.size() ? stream.http_body_pending : stream.read_buff.size();
		stream.read_buff.erase(stream.read_buff.begin(), stream.read_buff.begin() + discard_buff_size);
		stream.http_body_pending -= discard_buff_size;
	}

	if (stream.http_body_pending > 0) {
		stream.http_body_pending -= conn.read(stream.http_body_pending).size();
	}
}

bool Impl::is_connection_upgrade(const Headers& headers) {

	auto normalize_value = [](std::string value) -> std::string {
		for (auto& rune : value) {
			if (rune >= 'A' && rune <= 'Z') {
				rune += 0x20;
			}
		}
		return value;
	};

	return normalize_value(headers.get("connection")).contains("upgrade");
}
