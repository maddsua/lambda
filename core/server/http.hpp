#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "../network/tcp/connection.hpp"
#include "../network/tcp/listen.hpp"
#include "./server.hpp"

#include <future>
#include <optional>
#include <queue>

namespace Lambda::Server {

	enum struct ContentEncodings {
		None = 0,
		Brotli = 1,
		Gzip = 2,
		Deflate = 3,
	};

	struct RequestQueueItem {
		HTTP::Request request;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	class HttpRequestQueue {
		private:
			std::future<void> m_reader;
			std::queue<RequestQueueItem> m_queue;
			std::mutex m_lock;

		public:
			HttpRequestQueue(Network::TCP::Connection& conn, const ServeOptions& options);
			~HttpRequestQueue();

			HttpRequestQueue& operator=(const HttpRequestQueue& other) = delete;
			HttpRequestQueue& operator=(HttpRequestQueue&& other) noexcept;

			bool await();
			RequestQueueItem next();
			void push(RequestQueueItem&& item);
	};

	class RequestQueue {
		private:
			std::queue<RequestQueueItem> m_queue;
			std::mutex m_mutex;
			bool m_done = false;

		public:
			RequestQueue() {}
			RequestQueue(const std::initializer_list<RequestQueueItem>& init);
			RequestQueue(const Lambda::Server::RequestQueue& other);
			bool await();
			bool hasNext() const noexcept;
			void push(const RequestQueueItem& item);
			void finish() noexcept;
			RequestQueueItem next();
	};

	void serveHTTP(Network::TCP::Connection&& conn, HTTPRequestCallback handler, const ServeOptions& options);
	HTTP::Response errorResponse(int statusCode, std::optional<std::string> errorMessage);

};

#endif
