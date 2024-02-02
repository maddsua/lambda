#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__

#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <vector>

#include "../../network/tcp/connection.hpp"
#include "../../websocket/websocket.hpp"

namespace Lambda::HTTPServer {
	class ConnectionContext;
};

namespace Lambda::WSServer {

	struct WebsocketContextInit {
		Network::TCP::Connection& conn;
		std::vector<uint8_t>& connbuff;
	};

	class WebsocketContext {
		private:
			Network::TCP::Connection& conn;
			std::future<void> m_reader;
			std::queue<Websocket::Message> m_queue;
			std::mutex m_read_lock;
			bool m_stopped = false;

		public:
			WebsocketContext(WebsocketContextInit init);
			~WebsocketContext();

			bool awaitMessage();
			bool hasMessage() const noexcept;
			Websocket::Message nextMessage();
			void sendMessage(const Websocket::Message& msg);
			void close(Websocket::CloseReason reason);
	};

};

#endif
