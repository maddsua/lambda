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
#include "../sse/sse.hpp"
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

		const std::function<SSE::Writer()>& startEventStream;
		const std::function<Websocket::WebsocketContext()>& upgrateToWebsocket;
	};

	typedef std::function<HTTP::Response(const HTTP::Request&, const RequestContext&)> ServerlessCallback;

	class LambdaInstance {
		private:

			Network::TCP::ListenSocket listener;
			ServerConfig config;
			ServerlessCallback httpHandler;

			std::future<void> watchdogWorker;
			bool m_terminated = false;

			void terminate();

		public:
			LambdaInstance(ServerlessCallback handlerCallback, ServerConfig init);
			~LambdaInstance();

			void shutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
