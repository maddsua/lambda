#ifndef __LAMBDA_SERVER_HANDLERS__
#define __LAMBDA_SERVER_HANDLERS__

#include <expected>

#include "./server.hpp"
#include "../http/http_private.hpp"

namespace Lambda::Pipelines {

	namespace H1 {

		void serve_conn(Net::TcpConnection&& conn, HandlerFn handler, ServerContext ctx);

		namespace Impl {

			const size_t header_chunk_size = 2048;
			const size_t body_chunk_size = 128 * 1024;

			const size_t nullidx = -1;

			struct RequestError {
				std::string message;
				Status status = Status::BadRequest;
				bool discard = false;
			};

			struct StreamState {
				HTTP::Buffer read_buff;
				size_t http_body_pending = 0;
				bool http_keep_alive = true;
				bool raw_io = false;
			};

			void serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, ServerContext ctx);

			std::expected<Request, RequestError> read_request_head(Net::TcpConnection& conn, HTTP::Buffer& read_buff, ServerContext ctx);
			bool method_can_have_body(Method method);
			HTTP::Buffer read_request_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size, size_t bytes_remain);
			HTTP::Buffer read_raw_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size);
			std::optional<size_t> content_length(const Headers& headers);

			size_t write_head(Net::TcpConnection& conn, int status, const Headers& headers);
			void write_request_error(Net::TcpConnection& conn, Status status, std::string message);
			void set_response_meta(Headers& headers);
			void discard_unread_body(Net::TcpConnection& conn, StreamState& stream);

			bool is_connection_upgrade(const Headers& headers);
		};
	};

};

#endif
