#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKET_TRANSPORT__

#include "./websocket.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <array>

namespace Lambda::Websocket::Transport {

	enum struct OpCode : uint8_t {
		Text = 0x01,
		Binary = 0x02,
		Close = 0x08,
		Ping = 0x09,
		Pong = 0x0A,
		Continue = 0x00,
	};

	enum struct FrameControlBits : uint8_t {
		BitFinal = 0x80,
		BitContinue = 0x00
	};

	struct FrameHeader {
		FrameControlBits finbit;
		OpCode opcode;
		size_t size;
		size_t payloadSize;
		static const size_t mask_size = 4;
		static const size_t min_size = 2;
		std::optional<std::array<uint8_t, mask_size>> mask;
	};

	struct MultipartMessageContext {
		std::array<uint8_t, FrameHeader::mask_size> mask;
		bool binary = false;
	};

	FrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer);
	std::vector <uint8_t> serializeMessage(const Websocket::Message& message);
	std::vector <uint8_t> serializeFrameHeader(const FrameHeader& header);
};

#endif
