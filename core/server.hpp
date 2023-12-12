#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	namespace Server {
		typedef std::function<HTTP::Response(const HTTP::Request request, const Network::ConnInfo& info)> HttpHandlerFunction;
		void handleHTTPConnection(Network::TCPConnection conn, HttpHandlerFunction handler);
	};

	struct HttpServerInit {
		uint16_t port = 8180;
		bool fastPortReuse = false;
	};

	class HttpServer {
		private:
			Network::TCPListenSocket* listener = nullptr;
			Server::HttpHandlerFunction handler;
			std::future<void> watchdogRoutine;
			HttpServerInit config;
			bool terminated = false;

		public:
			HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerInit init);
			~HttpServer();

			const HttpServerInit& getConfig() const noexcept;
	};

};

#endif
