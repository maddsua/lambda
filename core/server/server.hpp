#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>
#include <optional>

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "../network/tcp/connection.hpp"
#include "../network/tcp/listen.hpp"
#include "./console/handlerConsole.hpp"

namespace Lambda {

	struct ServerLogOptions {
		bool logConnections = false;
		bool logRequests = false;	
	};

	struct HTTPTransportOptions {
		bool useCompression = true;
		bool reuseConnections = true;
	};

	struct ServeOptions {
		ServerLogOptions loglevel;
		HTTPTransportOptions transport;
	};

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HttpHandlerFunction;
	void handleHTTPConnection(Network::TCP::Connection&& conn, HttpHandlerFunction handler, const ServeOptions& options);

	HTTP::Response serviceResponse(int statusCode, std::optional<std::string> errorMessage);

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};

	class Server {
		private:
			Network::TCP::ListenSocket* listener = nullptr;
			HttpHandlerFunction handler;
			ServerConfig config;
			std::thread watchdogWorker;
			bool terminated = false;

		public:
			Server(HttpHandlerFunction handlerFunction, ServerConfig init);
			~Server();

			void softShutdownn();
			void immediateShutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
