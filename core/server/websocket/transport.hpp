#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__

#include <vector>

#include "../../websocket/websocket.hpp"

namespace Lambda::WSServer::Transport {

	struct WebsocketFrameHeader {
		size_t payloadSize;
		uint8_t maskKey[4];
		uint8_t opcode;
		bool finbit;
		bool mask;
	};

	enum struct WebsockBits : uint8_t {
		BitFinal = 0x80,
		BitContinue = 0x00,
		Text = 0x01,
		Binary = 0x02,
		Close = 0x08,
		Ping = 0x09,
		Pong = 0x0A
	};

	WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer);
	std::vector <uint8_t> serializeMessage(const Websocket::Message& message);

};

#endif
