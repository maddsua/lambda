#include "./pipelines.hpp"
#include "../version.hpp"

#include <cstdint>

using namespace Lambda;
using namespace Lambda::Pipelines::H1;

void Pipelines::H1::serve_conn(Net::TcpConnection&& conn, HandlerFn handler, ServeContext ctx) {

	conn.set_timeouts({
		.read = Server::DefaultIoTimeout,
		.write = Server::DefaultIoTimeout
	});

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

void Impl::serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, const ServeContext& ctx) {

	auto request_body_reader = Impl::RequestBodyReader(conn, stream);
	auto response_writer = Impl::ResponseWriter(conn, stream, ctx);

	auto expected_req = Impl::read_request_head(conn, stream.read_buff, ctx);
	if (!expected_req.has_value()) {

		if (!conn.is_open()) {
			return;
		}

		const auto error = expected_req.error();
		if (error.discard) {
			conn.close();
			return;
		}

		response_writer.header().set("connection", "close");
		response_writer.header().set("content-type", "text/plain");
		response_writer.write_header(error.status);
		response_writer.write(error.message);

		response_writer.flush();
		conn.close();

		return;
	}

	auto req = std::move(expected_req.value());
	auto can_have_body = HTTP::method_can_have_body(req.method);
	auto has_content_type = req.headers.has("content-type");
	auto content_length = can_have_body ? Impl::content_length(req.headers) : std::nullopt;

	stream.http_body_pending = content_length.has_value() ? content_length.value() : 0;

	//	drop mutation requests with no content body
	if (can_have_body && has_content_type && !content_length.has_value()) {

		response_writer.header().set("content-type", "text/plain");
		response_writer.write_header(Status::LengthRequired);
		response_writer.write("Content-Length header required for methods POST, PUT, PATCH...");

		response_writer.flush();

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
		.url = req.url,
		.headers = req.headers,
		.cookies = HTTP::parse_cookie(req.headers.get("cookie")),
		.body = request_body_reader,
	};

	handler(request, response_writer);

	response_writer.flush();
}
