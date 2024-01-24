#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>
#include <optional>

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "./console/handlerConsole.hpp"

namespace Lambda {

	struct LogOptions {
		bool logConnections = false;
		bool logRequests = false;	
	};

	struct HTTPTransportOptions {
		bool useCompression = true;
		bool reuseConnections = true;
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTPTransportOptions transport;
	};

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HandlerFunction;

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};

	class ServerInstance {
		private:
			Network::TCP::ListenSocket* listener = nullptr;
			HandlerFunction handler;
			ServerConfig config;
			std::thread watchdogWorker;
			bool terminated = false;

		public:
			ServerInstance(HandlerFunction handlerCallback, ServerConfig init);
			~ServerInstance();

			void softShutdownn();
			void immediateShutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
