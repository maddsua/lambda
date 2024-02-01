#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::HTTPServer {

	struct ReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
		bool keepAlive = false;
	};

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	struct IncomingRequest {
		HTTP::Request request;
		std::string pathname;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	std::optional<IncomingRequest> requestReader(ReaderContext& ctx);
	void writeResponse(const HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings preferEncoding);

	class HttpRequestQueue {
		private:
			std::future<void> m_reader;
			ReaderContext ctx;
			std::queue<IncomingRequest> m_queue;
			std::mutex m_lock;

		public:
			HttpRequestQueue(Network::TCP::Connection& conn, const HTTPTransportOptions& options);
			~HttpRequestQueue();

			HttpRequestQueue& operator=(const HttpRequestQueue& other) = delete;
			HttpRequestQueue& operator=(HttpRequestQueue&& other) noexcept;

			bool await();
			IncomingRequest next();
			void push(IncomingRequest&& item);
	};

	struct ConnectionContext : ReaderContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	};

};

#endif
