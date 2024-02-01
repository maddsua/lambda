#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::Server::Handlers {

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

	struct RequestQueueItem {
		HTTP::Request request;
		std::string pathname;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	std::optional<RequestQueueItem> requestReader(ReaderContext& ctx);
	void writeResponse(HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings preferEncoding);

	void httpServerlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const HTTPRequestCallback& handlerCallback) noexcept;
	void httpExtendedHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;

	class HttpRequestQueue {
		private:
			std::future<void> m_reader;
			Server::Handlers::ReaderContext ctx;
			std::queue<RequestQueueItem> m_queue;
			std::mutex m_lock;

		public:
			HttpRequestQueue(Network::TCP::Connection& conn, const HTTPTransportOptions& options);
			~HttpRequestQueue();

			HttpRequestQueue& operator=(const HttpRequestQueue& other) = delete;
			HttpRequestQueue& operator=(HttpRequestQueue&& other) noexcept;

			bool await();
			RequestQueueItem next();
			void push(RequestQueueItem&& item);
	};
};

#endif
