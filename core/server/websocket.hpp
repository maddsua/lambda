#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_SERVER__

#include <functional>
#include <optional>
#include <future>

#include "../network/network.hpp"
#include "../websocket/websocket.hpp"

namespace Lambda::WSServer {

	struct WebsocketContext {
		bool awaitMessage() const noexcept;
		Websocket::Message nextMessage();
		void sendMessage(const Websocket::Message& msg);
		void close(Websocket::CloseReason reason);
	};

};

#endif
