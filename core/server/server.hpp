#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <optional>
#include <future>

#include "../network/network.hpp"
#include "../http/http.hpp"
#include "./websocket/websocket.hpp"
#include "./http/http.hpp"
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

	namespace HTTPServer {
		struct ConnectionContext;
	};

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;
	typedef std::function<void(HTTPServer::IncomingConnection&)> ConnectionCallback;

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
		uint32_t connectionTimeout = 15000;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};

	class ServerInstance {
		private:

			enum struct HandlerType {
				Undefined, Connection, Serverless
			};

			Network::TCP::ListenSocket listener;

			ServerlessCallback httpHandler;
			ConnectionCallback tcpHandler;

			ServerConfig config;
			HandlerType handlerType = HandlerType::Undefined;

			std::future<void> watchdogWorker;
			bool terminated = false;

			void start();
			void terminate();

		public:
			ServerInstance(ServerlessCallback handlerCallback, ServerConfig init);
			ServerInstance(ConnectionCallback handlerCallback, ServerConfig init);
			~ServerInstance();

			void shutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
