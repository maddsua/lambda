#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_WEBSOCKET__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_WEBSOCKET__

#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <mutex>
#include <future>

#include "../network/network.hpp"
//#include "../network/tcp/connection.hpp"
//#include "../http/http.hpp"
#include "../websocket/message.hpp"

namespace Lambda::Websocket {

	enum struct CloseCode : int {
		Normal = 1000,
		GoingAway = 1001,
		ProtocolError = 1002,
		UnsupportedData = 1003,
		NoStatusReceived = 1005,
		AbnormalClose = 1006,
		InvalidPayload = 1007,
		PolicyViolation = 1008,
		MessageTooBig = 1009,
		MandatoryExtension = 1010,
		InternalServerError = 1011,
		TLSHandshakeFailed = 1015
	};

	enum struct StreamTerminateFlags {
		None, Failed, Closed, Terminated
	};

	struct WebsocketInfo {
		std::string requestID;
	};

	class WebsocketStream {
		private:
			Lambda::Network::TCP::Connection* conn = nullptr;
			StreamTerminateFlags terminateFlags = StreamTerminateFlags::None;
			std::queue<Message> rxQueue;
			std::mutex readMutex;
			std::vector<uint8_t> txQueue;
			std::mutex writeMutex;
			std::future<void> ioworker;
			WebsocketInfo info;

		public:
			WebsocketStream(Lambda::Network::TCP::Connection& conn, const WebsocketInfo& infoInit);
			~WebsocketStream();

			bool available() const noexcept;
			bool ok() const noexcept;
			Message getMessage();
			void sendMessage(const Message& msg);
			void terminate();
			void close();
			void close(CloseCode reason);
	};
};

#endif
