#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <string>

namespace Lambda::Server {

	namespace Handlers {
		void serverlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ServerlessCallback& handlerCallback) noexcept;
		void connectionHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;
	};

	namespace HTTPTransport {

		std::optional<IncomingRequest> requestReader(HTTPReaderContext& ctx);
		void writeResponse(const HTTP::Response& response, const HTTPWriterContext& ctx);

		std::optional<std::pair<std::string, std::string>> parseBasicAuth(const std::string& header);

	};

	namespace WSTransport {

		enum struct OpCode : uint8_t {
			Text = 0x01,
			Binary = 0x02,
			Close = 0x08,
			Ping = 0x09,
			Pong = 0x0A,
			Continue = 0x00,
		};

		enum struct WebsockBits : uint8_t {
			BitFinal = 0x80,
			BitContinue = 0x00
		};

		struct WebsocketFrameHeader {
			WebsockBits finbit;
			OpCode opcode;
			size_t size;
			size_t payloadSize;
			static const size_t mask_size = 4;
			static const size_t min_size = 2;
			std::optional<std::array<uint8_t, mask_size>> mask;
		};

		struct MultipartMessageContext {
			std::array<uint8_t, WebsocketFrameHeader::mask_size> mask;
			bool binary = false;
		};

		WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer);
		std::vector <uint8_t> serializeMessage(const Websocket::Message& message);
		std::vector <uint8_t> serializeFrameHeader(const WebsocketFrameHeader& header);

	};
};

#endif
