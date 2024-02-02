#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__

#include <functional>
#include <future>
#include <queue>
#include <mutex>

#include "../network/network.hpp"
#include "../websocket/websocket.hpp"

namespace Lambda::HTTPServer {
	class ConnectionContext;
};

namespace Lambda::WSServer {

	class WebsocketContext {
		private:
			HTTPServer::ConnectionContext* connctx = nullptr;
			std::future<void> m_reader;
			std::queue<Websocket::Message> m_queue;
			std::mutex m_read_lock;

		public:
			WebsocketContext(HTTPServer::ConnectionContext* connctxInit);
			~WebsocketContext();

			bool awaitMessage();
			bool hasMessage() const noexcept;
			Websocket::Message nextMessage();
			void sendMessage(const Websocket::Message& msg);
			void close(Websocket::CloseReason reason);
	};

};

#endif
