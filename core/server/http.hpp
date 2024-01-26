#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HTTPSERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HTTPSERVER__

#include "../network/tcp/connection.hpp"
#include "../network/tcp/listener.hpp"
#include "../network/network.hpp"
#include "./server.hpp"

#include <future>
#include <queue>

namespace Lambda::HTTPServer {

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	struct RequestQueueItem {
		HTTP::Request request;
		std::string pathname;
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	class HttpRequestQueue {
		private:
			std::future<void> m_reader;
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

	void connectionHandler(Network::TCP::Connection&& conn, HTTPRequestCallback handlerCallback, const ServerConfig& config) noexcept;
	void writeResponse(HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings useEncoding);
	void asyncReader(Network::TCP::Connection& conn, const HTTPTransportOptions& options, HttpRequestQueue& queue);

};

#endif
