#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <optional>
#include <queue>
#include <future>

#include "../network/network.hpp"
#include "../network/tcp/listener.hpp"
#include "../http/transport.hpp"
#include "../http/http.hpp"
#include "../websocket/websocket.hpp"
#include "../crypto/crypto.hpp"
#include "../utils/utils.hpp"

namespace Lambda {

	struct LogOptions {
		bool transportEvents = false;
		bool requests = true;
		bool startMessage = true;
	};

	enum struct ErrorResponseType {
		HTML, JSON
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTP::Transport::TransportOptions transport;
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
		const std::string& contextID;
		const std::string& requestID;
		const Network::ConnectionInfo& conninfo;
	};

	struct IncomingConnection {
		private:

			enum struct ActiveProtocol {
				HTTP, WS
			};

			Crypto::ShortID m_ctx_id;

			Network::TCP::Connection& conn;
			const ServeOptions& opts;
			HTTP::Transport::V1TransportContext ctx;
			ActiveProtocol activeProto = ActiveProtocol::HTTP;

		public:
			IncomingConnection(Network::TCP::Connection& connInit, const ServeOptions& optsInit);

			IncomingConnection(const IncomingConnection& other) = delete;
			IncomingConnection& operator=(const IncomingConnection& other) = delete;

			const Crypto::ShortID& contextID() const noexcept;
			Network::TCP::Connection& tcpconn() const noexcept;
			const Network::ConnectionInfo& conninfo() const noexcept;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);

			Websocket::WebsocketContext upgrateToWebsocket();
			Websocket::WebsocketContext upgrateToWebsocket(const HTTP::Request& initialRequest);
	};	

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;
	typedef std::function<void(IncomingConnection&)> ConnectionCallback;

	class LambdaInstance {
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
			bool m_terminated = false;

			void start();
			void terminate();

		public:
			LambdaInstance(ServerlessCallback handlerCallback, ServerConfig init);
			LambdaInstance(ConnectionCallback handlerCallback, ServerConfig init);
			~LambdaInstance();

			void shutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
