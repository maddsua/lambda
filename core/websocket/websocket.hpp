#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__

#include "../network/tcp/connection.hpp"
#include "../http/transport.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <future>
#include <queue>

namespace Lambda::Websocket {

	struct Message {
		const std::vector<uint8_t> data;
		const bool binary = false;
		const bool partial = false;
		const time_t timestamp = 0;

		Message(const std::string& dataInit);
		Message(const std::string& dataInit, bool partial);
		Message(const std::vector<uint8_t>& dataInit);
		Message(const std::vector<uint8_t>& dataInit, bool partial);
		Message(const std::vector<uint8_t>& dataInit, bool binaryInit, bool partialInit);

		std::string text() const noexcept;
		size_t size() const noexcept;
	};

	enum struct CloseReason : int16_t {
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

	class WebsocketContext {
		private:
			HTTP::Transport::TransportContext& transport;
			const HTTP::Transport::TransportOptions& topts;
			std::future<void> m_reader;
			std::queue<Websocket::Message> m_queue;
			std::mutex m_read_lock;
			bool m_stopped = false;
			void asyncWorker();

		public:

			WebsocketContext(HTTP::Transport::TransportContext& tctx, const HTTP::Transport::IncomingRequest& initRequest);
			~WebsocketContext();

			bool awaitMessage();
			bool hasMessage() const noexcept;
			Websocket::Message nextMessage();
			void sendMessage(const Websocket::Message& msg);
			void close(Websocket::CloseReason reason);
	};
};

#endif
