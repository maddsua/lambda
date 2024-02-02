#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <optional>
#include <future>

#include "../http/http.hpp"
#include "../network/network.hpp"
#include "./console.hpp"
#include "./websocket.hpp"

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

	struct IncomingConnection {
		private:

			enum struct ActiveProtocol {
				HTTP, WS
			};

			HTTPServer::ConnectionContext* ctx = nullptr;
			ActiveProtocol activeProto = ActiveProtocol::HTTP;

		public:
			IncomingConnection(Network::TCP::Connection* conn, const HTTPTransportOptions& opts);
			~IncomingConnection();

			IncomingConnection(const IncomingConnection& other) = delete;
			IncomingConnection& operator=(const IncomingConnection& other) = delete;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);

			WSServer::WebsocketContext upgrateToWebsocket();
	};

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;
	typedef std::function<void(IncomingConnection&)> ConnectionCallback;

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};

	class ServerInstance {
		private:

			enum struct HandlerType {
				Undefined, Connection, Serverless
			};

			Network::TCP::ListenSocket* listener = nullptr;

			ServerlessCallback httpHandler;
			ConnectionCallback tcpHandler;

			ServerConfig config;
			HandlerType handlerType = HandlerType::Undefined;

			std::future<void> watchdogWorker;
			bool terminated = false;

			void terminate();
			void setup();

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
