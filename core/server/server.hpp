#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <optional>
#include <queue>
#include <future>

#include "./console.hpp"
#include "../network/network.hpp"
#include "../network/tcp/listener.hpp"
#include "../http/http.hpp"
#include "../websocket/websocket.hpp"

namespace Lambda {

	struct LogOptions {
		bool connections = true;
		bool requests = true;
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

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
		Network::ConnectionTimeouts connectionTimeouts;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	struct HTTPReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
		bool keepAlive = false;
	};

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	struct HTTPTransportContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	struct IncomingRequest : HTTPTransportContext {
		HTTP::Request request;
	};

	struct HTTPWriterContext : HTTPTransportContext {
		Network::TCP::Connection& conn;
	};

	struct ConnectionContext : HTTPReaderContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	};

	class WebsocketContext {
		private:
			Network::TCP::Connection& conn;
			std::future<void> m_reader;
			std::queue<Websocket::Message> m_queue;
			std::mutex m_read_lock;
			bool m_stopped = false;

		public:

			struct ContextInit {
				Network::TCP::Connection& conn;
				std::vector<uint8_t>& connbuff;
			};

			WebsocketContext(ContextInit init);
			~WebsocketContext();

			bool awaitMessage();
			bool hasMessage() const noexcept;
			Websocket::Message nextMessage();
			void sendMessage(const Websocket::Message& msg);
			void close(Websocket::CloseReason reason);
	};

	struct IncomingConnection {
		private:

			enum struct ActiveProtocol {
				HTTP, WS
			};

			ConnectionContext ctx;
			ActiveProtocol activeProto = ActiveProtocol::HTTP;

		public:
			IncomingConnection(Network::TCP::Connection& conn, const HTTPTransportOptions& opts);

			IncomingConnection(const IncomingConnection& other) = delete;
			IncomingConnection& operator=(const IncomingConnection& other) = delete;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);

			WebsocketContext upgrateToWebsocket();
			WebsocketContext upgrateToWebsocket(const HTTP::Request& initialRequest);
	};	

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;
	typedef std::function<void(IncomingConnection&)> ConnectionCallback;

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
