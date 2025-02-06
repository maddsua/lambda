#include "./pipelines.hpp"
#include "../version.hpp"

#include <cstdint>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

bool is_connection_upgrade(const Headers& headers);
bool is_streaming_response(const Headers& headers);

struct DeferredResponse {
	Headers headers;
	Status status;
	HTTP::Buffer body;
};

class ResponseWriterImpl : public ResponseWriter {
	private:
		Net::TcpConnection& m_conn;
		Impl::StreamState& m_stream;
		Headers m_headers;
		bool m_header_written = false;
		size_t m_body_written = 0;
		std::optional<DeferredResponse> m_deferred;
		std::optional<size_t> m_announced;

	public:
		ResponseWriterImpl(Net::TcpConnection& conn, Impl::StreamState& stream)
			: m_conn(conn), m_stream(stream) {}

		~ResponseWriterImpl() {

			if (!this->m_conn.is_open()) {
				return;
			}

			if (!this->m_header_written) {
				this->m_headers.set("content-length", std::to_string(0));
				Impl::write_head(this->m_conn, Status::OK, this->m_headers);
				return;
			}

			if (this->m_deferred.has_value()) {

				auto& deferred = this->m_deferred.value();
				
				//	set content length to terminate response
				deferred.headers.set("content-length", std::to_string(deferred.body.size()));

				Impl::write_head(this->m_conn, deferred.status, deferred.headers);			
				this->m_conn.write(deferred.body);
			}
		}

		bool writable() const noexcept {
			return this->m_conn.is_open();
		}

		Headers& header() noexcept {
			return this->m_headers;
		}

		size_t write_header() {
			return this->write_header(Status::OK);
		}

		size_t write_header(Status status) {

			if (this->m_header_written) {
				throw std::runtime_error("Response header has been already written");
			}

			this->m_header_written = true;

			//	enable raw io for connections that were upgraded (eg to websockets)
			if (is_connection_upgrade(this->m_headers)) {
				
				this->m_headers.del("content-length");

				this->m_stream.http_keep_alive = false;
				this->m_stream.http_body_pending = 0;
				this->m_stream.raw_io = true;

				//	set raw read timeout to a small value
				this->m_conn.set_timeouts({ .read = Impl::raw_io_read_timeout, .write = this->m_conn.timeouts().write });
			}

			//	override keep-alive for streaming connections such as sse
			if (is_streaming_response(this->m_headers)) {
				this->m_headers.del("content-length");
				this->m_stream.http_keep_alive = false;
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
			auto content_length = this->m_headers.get("content-length");
			if (content_length.empty()) {

				this->m_deferred = DeferredResponse {
					.headers = std::move(this->m_headers),
					.status = status,
				};

				this->m_headers = {};

				return 0;
			}

			this->m_announced = std::stoul(content_length);

			return Impl::write_head(this->m_conn, status, this->m_headers);
		}

		size_t write(const HTTP::Buffer& data) {

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

				//	todo: write to debug when response is truncated

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

		size_t write(const std::string& text) {
			return this->write(HTTP::Buffer(text.begin(), text.end()));
		}

		void set_cookie(const Cookie& cookie) {
			this->header().append("Set-Cookie", cookie.to_string());
		}

		bool is_deferred() const noexcept {
			return this->m_deferred.has_value();
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

	auto request_body_reader = RequestBodyImpl(conn, stream);
	auto response_writer = ResponseWriterImpl(conn, stream);

	auto expected_req = Impl::read_request_head(conn, stream.read_buff, ctx);
	if (!expected_req.has_value()) {

		if (!conn.is_open()) {
			return;
		}

		const auto error = expected_req.error();
		if (error.discard) {
			return;
		}

		response_writer.header().set("connection", "close");
		response_writer.header().set("content-type", "text/plain");
		response_writer.write_header(error.status);
		response_writer.write(error.message);

		return;
	}

	auto req = std::move(expected_req.value());
	auto can_have_body = HTTP::method_can_have_body(req.method);
	auto has_content_type = req.headers.has("content-type");
	auto content_length = can_have_body ? Impl::content_length(req.headers) : std::nullopt;

	stream.http_body_pending = content_length.has_value() ? content_length.value() : 0;

	//	drop mutation requests with no content body
	if (can_have_body && has_content_type && !content_length.has_value()) {

		response_writer.header().set("connection", "close");
		response_writer.header().set("content-type", "text/plain");
		response_writer.write_header(Status::LengthRequired);
		response_writer.write("Content-Length header required for methods POST, PUT, PATCH...");

		return;
	}

	//	get keepalive from client
	auto client_connection_header = req.headers.get("connection");
	if (client_connection_header.contains("close")) {
		stream.http_keep_alive = false;
	}

	auto request = Request {
		.remote_addr = conn.remote_addr(),
		.method = req.method,
		.url = std::move(req.url),
		.headers = std::move(req.headers),
		.cookies = HTTP::parse_cookie(req.headers.get("cookie")),
		.body = request_body_reader,
	};

	handler(request, response_writer);
}

bool is_connection_upgrade(const Headers& headers) {
	return HTTP::reset_case(headers.get("connection")).contains("upgrade");
}

bool is_streaming_response(const Headers& headers) {
	return HTTP::reset_case(headers.get("content-type")).contains("event-stream");
}
