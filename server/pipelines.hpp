#ifndef __LIB_MADDSUA_LAMBDA_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_SERVER_HANDLERS__

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
				bool stream_response = false;
			};

			struct RequestHead {
				Method method;
				URL url;
				Headers headers;
			};

			struct DeferredResponse {
				Headers headers;
				Status status;
				HTTP::Buffer body;
			};

			void serve_request(Net::TcpConnection& conn, HandlerFn handler, StreamState& stream, ServeContext ctx);

			std::expected<RequestHead, RequestError> read_request_head(Net::TcpConnection& conn, HTTP::Buffer& read_buff, ServeContext ctx);
			HTTP::Buffer read_request_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size, size_t bytes_remain);
			HTTP::Buffer read_raw_body(Net::TcpConnection& conn, HTTP::Buffer& read_buff, size_t chunk_size);
			std::optional<size_t> content_length(const Headers& headers);

			size_t write_head(Net::TcpConnection& conn, Status status, const Headers& headers);
			void discard_unread_body(Net::TcpConnection& conn, StreamState& stream);

			class ResponseWriter : public Lambda::ResponseWriter {
				private:
					Net::TcpConnection& m_conn;
					Impl::StreamState& m_stream;
					Headers m_headers;
					bool m_header_written = false;
					size_t m_body_written = 0;
					std::optional<Impl::DeferredResponse> m_deferred;
					std::optional<size_t> m_announced;

				public:
					ResponseWriter(Net::TcpConnection& conn, StreamState& stream)
						: m_conn(conn), m_stream(stream) {}

					~ResponseWriter();

					bool writable() const noexcept;

					Headers& header() noexcept;

					size_t write_header();
					size_t write_header(Status status);

					size_t write(const HTTP::Buffer& data);
					size_t write(const std::string& text);
					void set_cookie(const Cookie& cookie);
					bool is_deferred() const noexcept;
			};

			class RequestBodyReader : public Lambda::BodyReader {
				private:
					Net::TcpConnection& m_conn;
					Impl::StreamState& m_stream;

				public:
					RequestBodyReader(Net::TcpConnection& conn, StreamState& stream)
						: m_conn(conn), m_stream(stream) {}

					bool is_readable() const noexcept;

					HTTP::Buffer read(size_t chunk_size);
					HTTP::Buffer read_all();
					std::string text();
			};
		};
	};

};

#endif
