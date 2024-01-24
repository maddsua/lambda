#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_WEBSOCKET__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_WEBSOCKET__

#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <mutex>
#include <future>

#include "../network/network.hpp"
#include "../websocket/websocket.hpp"

namespace Lambda::Websocket {

	class WebsocketStream {
		private:
			Lambda::Network::TCP::Connection* conn = nullptr;
			std::queue<Message> rxQueue;
			std::mutex readMutex;
			std::vector<uint8_t> txQueue;
			std::mutex writeMutex;
			std::future<void> ioworker;

		public:
			bool available() const noexcept;
			bool ok() const noexcept;
			Message getMessage();
			void sendMessage(const Message& msg);
			void close();
			void close(CloseCode reason);
	};
};

#endif
