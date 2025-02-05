#ifndef __LAMBDA_SERVER_HANDLERS__
#define __LAMBDA_SERVER_HANDLERS__

#include <expected>

#include "./server.hpp"
#include "../http/http_utils.hpp"

namespace Lambda::Pipelines {

	namespace H1 {

		void serve_conn(Net::TcpConnection&& conn, HandlerFn handler, ServeContext ctx);

		namespace Impl {

			const size_t header_chunk_size = 2048;
			const size_t body_chunk_size = 128 * 1024;
			const time_t raw_io_read_timeout = 100;

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

			struct RequestHead {
				Method method;
				URL url;
				Headers headers;
			};

			void serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, ServeContext ctx);

			std::expected<RequestHead, RequestError> read_request_head(Net::TcpConnection& conn, HTTP::Buffer& read_buff, ServeContext ctx);
			HTTP::Buffer read_request_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size, size_t bytes_remain);
			HTTP::Buffer read_raw_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size);
			std::optional<size_t> content_length(const Headers& headers);

			size_t write_head(Net::TcpConnection& conn, Status status, const Headers& headers);
			void terminate_with_error(Net::TcpConnection& conn, Status status, std::string message);
			void discard_unread_body(Net::TcpConnection& conn, StreamState& stream);
		};
	};

};

#endif
