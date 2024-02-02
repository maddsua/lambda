#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__

#include <vector>
#include <optional>
#include <array>

#include "../../websocket/websocket.hpp"

namespace Lambda::WSServer::Transport {

	enum struct OpCode : uint8_t {
		Text = 0x01,
		Binary = 0x02,
		Close = 0x08,
		Ping = 0x09,
		Pong = 0x0A
	};

	enum struct WebsockBits : uint8_t {
		BitFinal = 0x80,
		BitContinue = 0x00
	};

	struct WebsocketFrameHeader {
		WebsockBits finbit;
		OpCode opcode;
		size_t payloadSize;
		static const size_t mask_size = 4;
		std::optional<std::array<uint8_t, mask_size>> mask;
	};

	WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer);
	std::vector <uint8_t> serializeMessage(const Websocket::Message& message);
	std::vector <uint8_t> serializeFrameHeader(const WebsocketFrameHeader& header);

};

#endif
