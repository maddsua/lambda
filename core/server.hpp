#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>
#include <optional>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	namespace Server {

		struct HttpHandlerOptions {
			bool reuqestLoggingEnabled = true;
			bool errorLoggingEnabled = true;
			bool httpCompressionEnabled = true;
		};

		struct RequestContext {
			std::string requestID;
			Network::ConnInfo conninfo;
		};

		typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HttpHandlerFunction;
		void handleHTTPConnection(Network::TCPConnection&& conn, HttpHandlerFunction handler, const HttpHandlerOptions& options);

		HTTP::Response serviceResponse(int statusCode, std::optional<std::string> errorMessage);
	};

	struct HttpServerInit {
		Server::HttpHandlerOptions handlerOptions;
		uint16_t port = 8180;
		bool fastPortReuse = false;
	};

	class HttpServer {
		private:
			Network::TCPListenSocket* listener = nullptr;
			Server::HttpHandlerFunction handler;
			std::thread watchdogWorker;
			HttpServerInit config;
			bool terminated = false;

		public:
			HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerInit init);
			~HttpServer();

			void softShutdownn();
			void immediateShutdownn();
			void awaitFinished();

			const HttpServerInit& getConfig() const noexcept;
	};

	typedef HTTP::Request Request;
	typedef Server::RequestContext Context;

};

#endif
