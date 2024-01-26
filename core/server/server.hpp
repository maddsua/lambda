#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <thread>

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "./console.hpp"

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

	enum struct ErrorResponseType {
		HTML, JSON
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTPTransportOptions transport;
		ErrorResponseType errorResponseType = ErrorResponseType::HTML;
	};

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> HTTPRequestCallback;

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
			HTTPRequestCallback handler;
			ServerConfig config;
			std::thread watchdogWorker;
			bool terminated = false;

		public:
			ServerInstance(HTTPRequestCallback handlerCallback, ServerConfig init);
			~ServerInstance();

			void softShutdownn();
			void immediateShutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
