#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>
#include <optional>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	namespace Server {

		struct LoggingOptions {
			bool logConnections = false;
			bool logRequests = true;			
		};

		struct TransportOptions {
			bool httpCompressionEnabled = true;
		};

		struct HttpHandlerOptions {
			LoggingOptions loglevel;
			TransportOptions transport;
			std::string contextID;
		};

		struct RequestContext {
			std::string requestID;
			Network::ConnInfo conninfo;
		};

		typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HttpHandlerFunction;
		void handleHTTPConnection(Network::TCPConnection&& conn, HttpHandlerFunction handler, const HttpHandlerOptions& options);

		HTTP::Response serviceResponse(int statusCode, std::optional<std::string> errorMessage);

		struct ServiceOptions {
			uint16_t port = 8180;
			bool fastPortReuse = false;
		};

		struct HttpServerConfig {
			Server::LoggingOptions loglevel;
			Server::TransportOptions transport;
			ServiceOptions service;
		};

		class HttpServer {
			private:
				Network::TCPListenSocket* listener = nullptr;
				Server::HttpHandlerFunction handler;
				HttpServerConfig config;
				std::thread watchdogWorker;
				bool terminated = false;

			public:
				HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerConfig init);
				~HttpServer();

				void softShutdownn();
				void immediateShutdownn();
				void awaitFinished();

				const HttpServerConfig& getConfig() const noexcept;
		};
	};

	typedef Server::HttpServer HttpServer;
	typedef Server::HttpServerConfig HttpServerConfig;

	typedef HTTP::Request Request;
	typedef Server::RequestContext Context;

};

#endif
