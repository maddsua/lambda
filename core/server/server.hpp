#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <thread>

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "./handlers.hpp"

namespace Lambda {

	struct LogOptions {
		bool connections = false;
		bool requests = false;
		bool timestamps = false;
	};

	struct HTTPTransportOptions {
		bool useCompression = true;
		bool reuseConnections = true;
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTPTransportOptions transport;
	};

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
			Server::Handlers::HandlerFunction handler;
			ServerConfig config;
			std::thread watchdogWorker;
			bool terminated = false;

		public:
			ServerInstance(Server::Handlers::HandlerFunction handlerCallback, ServerConfig init);
			~ServerInstance();

			void softShutdownn();
			void immediateShutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
