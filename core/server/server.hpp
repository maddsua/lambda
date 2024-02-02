#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <optional>
#include <future>

#include "./options.hpp"
#include "../network/network.hpp"
#include "../network/tcp/listener.hpp"
#include "../http/http.hpp"
#include "./http/http.hpp"
#include "./websocket/websocket.hpp"
#include "./console.hpp"

namespace Lambda {

	struct RequestContext {
		std::string requestID;
		Network::ConnectionInfo conninfo;
		Console console;
	};

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;
	typedef std::function<void(HTTPServer::IncomingConnection&)> ConnectionCallback;

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
