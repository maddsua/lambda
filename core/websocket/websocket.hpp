#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__

#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <mutex>

#include "../network/network.hpp"

namespace Lambda::Websocket {

	enum struct CloseCode {
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

	class Message {
		private:
			std::vector<uint8_t> m_buffer;
			bool m_binary;
			bool m_partial;
			time_t m_timestamp;

		public:
			Message(const std::string& init);
			Message(const std::string& init, bool partial);
			Message(const std::vector<uint8_t>& init);
			Message(const std::vector<uint8_t>& init, bool partial);
			const std::vector<uint8_t>& data() const noexcept;
			std::string text() const;
			bool isBinary() const noexcept;
			bool isPartial() const noexcept;
			size_t size() const noexcept;
			time_t timstamp()const noexcept;
	};

	class WebsocketStream {
		private:
			Lambda::Network::TCP::Connection* conn = nullptr;
			std::queue<Message> rxQueue;
			std::mutex readMutex;
			std::queue<Message> txQueue;
			std::mutex writeMutex;

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
