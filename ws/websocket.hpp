#ifndef __LIB_MADDSUA_LAMBDA_WS__
#define __LIB_MADDSUA_LAMBDA_WS__

#include <cstdint>
#include <vector>
#include <optional>
#include <array>

#include "../http/http.hpp"

namespace Lambda {

	namespace Ws {

		enum struct Opcode : uint8_t {
			Continue = 0x00,
			Text = 0x01,
			Binary = 0x02,
			Close = 0x08,
			Ping = 0x09,
			Pong = 0x0A
		};

		enum struct FrameBit : uint8_t {
			Continue = 0x00,
			Final = 0x80
		};

		typedef std::array<uint8_t, 4> FrameMask;

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
	};

	struct Frame {
		Ws::FrameBit frame = Ws::FrameBit::Final;
		Ws::Opcode code = Ws::Opcode::Binary;
		std::vector<uint8_t> data;
	};

	struct MultipartState {
		Ws::Opcode code;
		std::optional<Ws::FrameMask> mask;
	};

	class Websocket {
		private:
			std::optional<MultipartState> m_multipart;
			std::vector<uint8_t> m_read_buff;
			BodyReader& m_reader;
			ResponseWriter& m_writer;
			bool m_closed = false;

		public:
			Websocket(Request& req, ResponseWriter& wtr);
			~Websocket();

			size_t write(const Frame& frame);
			size_t write(const std::string& message);
			size_t write(const std::vector<uint8_t>& data);
			std::optional<Frame> next();
			size_t close(Ws::CloseReason reason);
			bool is_open() const noexcept;
	};

};

#endif
