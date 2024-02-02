#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__

#include <cstdint>
#include <string>
#include <vector>

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
		std::string text() const noexcept;
		size_t size() const noexcept;
	};

	enum struct CloseReason : int {
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
};

#endif
