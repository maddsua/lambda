#include "./pipelines.hpp"

#include <cstdint>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

void Pipelines::H1::serve_conn(Net::TcpConnection&& conn, HandlerFn handler, ServerContext ctx) {

	//	todo: fix stream not disconnecting on idle
	conn.set_timeouts({
		.read = Server::DefaultIoTimeout,
		.write = Server::DefaultIoTimeout
	});

	Impl::StreamState stream;

	if (ctx.options().debug) {
		fprintf(stderr, "%s DEBUG Lambda::Serve::H1 { remote_addr='%s', conn=%i }: Create stream\n",
			Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id());
	}

	try {

		while (conn.is_open() && !ctx.done() && stream.http_keep_alive) {
			Impl::discard_unread_body(conn, stream);
			serve_request(conn, handler, stream, ctx);
		}

	} catch(const std::exception& e) {
		if (ctx.options().debug) {
			fprintf(stderr, "%s ERROR Lambda::Serve::H1 { remote_addr='%s', conn=%i }: H1 transport exception: %s\n",
				Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id(), e.what());
		}
	}

	if (ctx.options().debug) {
		fprintf(stderr, "%s DEBUG Lambda::Serve::H1 { remote_addr='%s', conn=%i }: Stream done\n",
			Date().to_log_string().c_str(), conn.remote_addr().hostname.c_str(), conn.id());
	}
}

void Impl::serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, ServerContext ctx) {

	auto expected_req = Impl::read_request_head(conn, stream.read_buff, ctx);
	if (!expected_req.has_value()) {
		
		if (!conn.is_open()) {
			return;
		}

		const auto error = expected_req.error();
		if (error.discard) {
			return;
		}

		return Impl::write_request_error(conn, error.status, error.message);
	}

	auto req = std::move(expected_req.value());

	auto can_have_body = Impl::method_can_have_body(req.method);
	auto has_content_type = req.headers.has("content-type");
	auto content_length = can_have_body ? Impl::content_length(req.headers) : std::nullopt;
	stream.http_body_pending = content_length.has_value() ? content_length.value() : 0;

	//	drop mutation requests with no content body
	if (can_have_body && has_content_type && !content_length.has_value()) {
		return Impl::write_request_error(conn, Status::LengthRequired, "Content-Length header required for methods POST, PUT, PATCH...");
	}

	//	get keepalive from client
	auto client_keep_alive = req.headers.get("connection");
	if (client_keep_alive.contains("close")) {
		stream.http_keep_alive = false;
	}

	bool header_written = false;

	auto body_reader = [&](size_t chunk_size) -> HTTP::Buffer {

		//	patch chunk size
		if (chunk_size == HTTP::LengthUnknown || chunk_size <= 0) {
			chunk_size = Impl::body_chunk_size;
		}

		//	stream reads (stuff like websockets)
		if (stream.raw_io) {
			return Impl::read_raw_body(conn, stream.read_buff, chunk_size);
		}

		//	return early if body is empty or consoomed
		if (stream.http_body_pending <= 0) {
			return {};
		}

		//	return body from streams
		auto chunk = Impl::read_request_body(conn, stream.read_buff, chunk_size, stream.http_body_pending);
		stream.http_body_pending -= chunk.size();

		return chunk;
	};

	Headers response_headers;
	response_headers.set("connection", stream.http_keep_alive ? "keep-alive" : "close");

	auto write_header = [&](uint16_t status, const HTTP::Values& header) -> size_t {

		if (header_written) {
			throw std::runtime_error("Response header has been already written");
		}

		header_written = true;

		Impl::set_response_meta(response_headers);

		//	mark response end with responses with no body and no body indications
		if (!response_headers.has("content-type") || !response_headers.has("content-length")) {
			response_headers.set("content-length", std::to_string(0));
		}

		if (Impl::is_connection_upgrade(response_headers)) {
			stream.http_keep_alive = false;
			stream.http_body_pending = 0;
			stream.raw_io = true;
		}

		return Impl::write_head(conn, status, response_headers);
	};

	auto write_data = [&](const HTTP::Buffer& data) -> size_t {

		//	todo: handle over-content-length writes

		size_t bytes_written = 0;

		if (!header_written) {

			header_written = true;

			Impl::set_response_meta(response_headers);

			//	force content type on untyped data
			if (!response_headers.has("content-type")) {
				response_headers.set("content-type", "application/octet-stream");
			}

			bytes_written = Impl::write_head(conn, static_cast<std::underlying_type_t<Status>>(Status::OK), response_headers);
		}

		bytes_written += conn.write(data);

		return bytes_written;
	};

	//	invoke provided handler fn
	auto writer = ResponseWriter(response_headers, write_header, write_data);
	req.body = RequestBody(body_reader);
	
	handler(req, writer);

	//	terminate empty responses
	if (!header_written && conn.is_open()) {
		response_headers.set("content-length", std::to_string(0));
		Impl::set_response_meta(response_headers);
		Impl::write_head(conn, static_cast<std::underlying_type_t<Status>>(Status::OK), response_headers);
	}
}
