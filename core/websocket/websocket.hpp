#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET__

#include <string>
#include <vector>
#include <cstdint>

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

		public:
			Message(const std::string& init);
			Message(const std::vector<uint8_t>& init);
			const std::vector<uint8_t>& data() const noexcept;
			std::string text() const;
			bool isBinary() const noexcept;
	};

};

#endif
