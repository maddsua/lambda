#include "./pipelines.hpp"

#include <cstdint>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

bool is_connection_upgrade(const Headers& headers);
bool is_streaming_response(const Headers& headers);

struct RequestState {
	Impl::RequestHead req;
	bool can_have_body = false;
	bool has_content_type = false;
	bool header_written = false;
	std::optional<size_t> content_length;
	Headers response_headers;
};

class ResponseWriterImpl : public ResponseWriter {
	private:
		Net::TcpConnection& m_conn;
		Impl::StreamState& m_stream;
		RequestState& m_state;

	public:
		ResponseWriterImpl(Net::TcpConnection& conn, Impl::StreamState& stream, RequestState& state)
			: m_conn(conn), m_stream(stream), m_state(state) {}

		bool writable() const noexcept {
			return this->m_conn.is_open();
		}

		Headers& header() noexcept {
			return this->m_state.response_headers;
		}

		size_t write_header() {
			return this->write_header(Status::OK);
		}

		size_t write_header(Status status) {

			if (this->m_state.header_written) {
				throw std::runtime_error("Response header has been already written");
			}

			this->m_state.header_written = true;

			//	enable raw io for connections that were upgraded (eg to websockets)
			if (is_connection_upgrade(this->m_state.response_headers)) {
				this->m_stream.http_keep_alive = false;
				this->m_stream.http_body_pending = 0;
				this->m_stream.raw_io = true;
				//	set raw read timeout to a small value
				this->m_conn.set_timeouts({ .read = Impl::raw_io_read_timeout, .write = this->m_conn.timeouts().write });
			}

			//	override keep-alive for streaming connections such as sse
			if (is_streaming_response(this->m_state.response_headers)) {
				this->m_stream.http_keep_alive = false;
			}

			//	mark response end with responses with no body and no body indications
			if (!this->m_state.response_headers.has("content-type") && !this->m_state.response_headers.has("content-length")) {
				this->m_state.response_headers.set("content-length", std::to_string(0));
			}

			return Impl::write_head(this->m_conn, status, this->m_state.response_headers);
		}

		size_t write(const HTTP::Buffer& data) {

			//	todo: handle over-content-length writes

			size_t bytes_written = 0;

			if (!this->m_state.header_written) {

				//	force content type on untyped data
				if (!this->m_state.response_headers.has("content-type")) {
					this->m_state.response_headers.set("content-type", "application/octet-stream");
				}

				bytes_written = this->write_header(Status::OK);
				this->m_state.header_written = true;
			}

			bytes_written += this->m_conn.write(data);

			return bytes_written;
		}

		size_t write(const std::string& text) {
			return this->write(HTTP::Buffer(text.begin(), text.end()));
		}

		void set_cookie(const Cookie& cookie) {
			this->m_state.response_headers.append("Set-Cookie", cookie.to_string());
		}
};

class RequestBodyImpl : public BodyReader {
	private:
		Net::TcpConnection& m_conn;
		Impl::StreamState& m_stream;

	public:
		RequestBodyImpl(Net::TcpConnection& conn, Impl::StreamState& stream)
			: m_conn(conn), m_stream(stream) {}

		bool is_readable() const noexcept {
			return this->m_conn.is_open();
		}

		HTTP::Buffer read(size_t chunk_size) {

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

		HTTP::Buffer read_all() {
			return this->read(HTTP::LengthUnknown);
		}

		std::string text() {
			auto buffer = this->read_all();
			return std::string(buffer.begin(), buffer.end());
		}
};

void Pipelines::H1::serve_conn(Net::TcpConnection&& conn, HandlerFn handler, ServeContext ctx) {

	//	todo: fix stream not disconnecting on idle
	conn.set_timeouts({
		.read = Server::DefaultIoTimeout,
		.write = Server::DefaultIoTimeout
	});

	//	todo: fix broken pipe exit on client disconnect during write

	Impl::StreamState stream;

	if (ctx.opts.debug) {
		fprintf(stderr, "%s DEBUG Lambda::Serve::H1 { remote_addr='%s', conn=%i }: Create stream\n",
			Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id());
	}

	try {

		while (conn.is_open() && !ctx.done() && stream.http_keep_alive) {
			Impl::discard_unread_body(conn, stream);
			serve_request(conn, handler, stream, ctx);
		}

	} catch(const std::exception& e) {
		if (ctx.opts.debug) {
			fprintf(stderr, "%s ERROR Lambda::Serve::H1 { remote_addr='%s', conn=%i }: H1 transport exception: %s\n",
				Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id(), e.what());
		}
	}

	if (ctx.opts.debug) {
		fprintf(stderr, "%s DEBUG Lambda::Serve::H1 { remote_addr='%s', conn=%i }: Stream done\n",
			Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id());
	}
}

void Impl::serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, ServeContext ctx) {

	auto expected_req = Impl::read_request_head(conn, stream.read_buff, ctx);
	if (!expected_req.has_value()) {

		if (!conn.is_open()) {
			return;
		}

		const auto error = expected_req.error();
		if (error.discard) {
			return;
		}

		return Impl::terminate_with_error(conn, error.status, error.message);
	}

	RequestState state {
		.req = std::move(expected_req.value()),
		.can_have_body = HTTP::method_can_have_body(state.req.method),
		.has_content_type = state.req.headers.has("content-type"),
		.header_written = false,
		.content_length = state.can_have_body ? Impl::content_length(state.req.headers) : std::nullopt,
	};

	stream.http_body_pending = state.content_length.has_value() ? state.content_length.value() : 0;

	//	drop mutation requests with no content body
	if (state.can_have_body && state.has_content_type && !state.content_length.has_value()) {
		return Impl::terminate_with_error(conn, Status::LengthRequired, "Content-Length header required for methods POST, PUT, PATCH...");
	}

	//	get keepalive from client
	auto client_connection_header = state.req.headers.get("connection");
	if (client_connection_header.contains("close")) {
		stream.http_keep_alive = false;
	}

	state.response_headers.set("connection", stream.http_keep_alive ? "keep-alive" : "close");

	//	prepare handler arguments
	auto request_body_reader = RequestBodyImpl(conn, stream);
	auto response_writer = ResponseWriterImpl(conn, stream, state);

	auto request = Request {
		.remote_addr = conn.remote_addr(),
		.method = state.req.method,
		.url = state.req.url,
		.headers = state.req.headers,
		.cookies = HTTP::parse_cookie(state.req.headers.get("cookie")),
		.body = request_body_reader,
	};

	//	invoke provided handler fn
	handler(request, response_writer);

	//	terminate empty responses
	if (!state.header_written && conn.is_open()) {
		state.response_headers.set("content-length", std::to_string(0));
		Impl::write_head(conn, Status::OK, state.response_headers);
	}
}

bool is_connection_upgrade(const Headers& headers) {
	return HTTP::reset_case(headers.get("connection")).contains("upgrade");
}

bool is_streaming_response(const Headers& headers) {
	return HTTP::reset_case(headers.get("content-type")).contains("event-stream");
}
