#include "./websocket.hpp"
#include "../http/http_utils.hpp"
#include "../base64/base64.hpp"
#include "../hash/hash.hpp"

#include <array>
#include <optional>
#include <arpa/inet.h>

using namespace Lambda;
using namespace Lambda::Ws;

const size_t read_chunk_size = 32;
const size_t max_header_size = 16;

Websocket::Websocket(Request& req, ResponseWriter& wrt) : m_reader(req.body), m_writer(wrt) {

	auto terminate_with_error = [&](Status status, const std::string& message) {
		try { 
			wrt.header().set("content-length", std::to_string(message.size()));
			wrt.write_header(status);
			wrt.write(message);
		} catch(...) {}
	};

	if (req.method != Method::GET) {
		terminate_with_error(Status::MethodNotAllowed, "invalid ws handshake method");
		throw std::runtime_error("Websocket: Invalid handshake: Invalid http request method");
	}

	auto upgrade_header = HTTP::reset_case(req.headers.get("upgrade"));
	auto header_ws_key = HTTP::reset_case(req.headers.get("sec-websocket-key"));
	if (upgrade_header != "websocket" || header_ws_key.empty()) {
		terminate_with_error(Status::BadRequest, "invalid ws upgrade headers");
		throw std::runtime_error("Websocket: Invalid handshake: Invalid upgrade headers");
	}

	auto ws_accept_key = Encoding::Base64::encode(Hash::SHA1::text(
		"258EAFA5-E914-47DA-95CA-C5AB0DC85B11" +
		header_ws_key
	));

	wrt.header().set("connection", "upgrade");
	wrt.header().set("upgrade", "websocket");
	wrt.header().set("Sec-WebSocket-Accept", ws_accept_key);
	wrt.write_header(Status::SwitchingProtocols);
}

Websocket::~Websocket() {

	if (!this->m_writer.writable()) {
		return;
	}

	try { this->close(CloseReason::GoingAway); }
		catch(...) {}
}

std::vector<uint8_t> pack_paylen(size_t data_size) {

	if (data_size < 126) {
		return { static_cast<uint8_t>(data_size & 0x7F) };
	}

	if (data_size < 65535) {
		return {
			static_cast<uint8_t>(126),
			static_cast<uint8_t>((data_size >> 8) & 255),
			static_cast<uint8_t>(data_size & 255)
		};
	}

	std::vector<uint8_t> buff {
		static_cast<uint8_t>(127)
	};

	for (size_t idx = 0; idx < 8; idx++) {
		buff.push_back((data_size >> ((7 - idx) * 8)) & 0xFF);
	}

	return buff;
}

std::optional<size_t> unpack_paylen(const std::vector<uint8_t>& read_buff, size_t& seek) {

	if (read_buff.size() < seek) {
		return std::nullopt;
	}

	auto payload_len_bit = static_cast<uint8_t>(read_buff[seek - 1] & 0x7F);

	if (payload_len_bit == 126) {

		uint16_t payload_len = 0;
		if (read_buff.size() < seek + sizeof(payload_len)) {
			return std::nullopt;
		}

		for (size_t idx = 0; idx < sizeof(payload_len); idx++) {
			payload_len |= (read_buff[seek + idx] << (((sizeof(payload_len) - 1) - idx) * 8));
		}

		seek += sizeof(payload_len);

		return payload_len;

	} else if (payload_len_bit == 127) {

		//	todo: test

		uint64_t payload_len = 0;
		if (read_buff.size() < seek + sizeof(payload_len)) {
			return std::nullopt;
		}

		for (size_t idx = 0; idx < sizeof(payload_len); idx++) {
			payload_len |= (read_buff[seek + idx] << (((sizeof(payload_len) - 1) - idx) * 8));
		}

		seek += sizeof(payload_len);

		return payload_len;
	}

	return payload_len_bit;
}

std::optional<Ws::FrameMask> unpack_mask(const std::vector<uint8_t>& read_buff, size_t& seek) {

	Ws::FrameMask mask_buff;

	if (read_buff.size() < seek + mask_buff.size()) {
		return std::nullopt;
	}

	for (size_t idx = 0; idx < mask_buff.size(); idx++) {
		mask_buff[idx] = read_buff[seek + idx];
	}

	seek += mask_buff.size();

	return mask_buff;
}

bool is_valid_frame_bit(FrameBit frame_bit) {
	return frame_bit == FrameBit::Continue || frame_bit == FrameBit::Final;
}

bool is_valid_opcode(Opcode opcode) {
	return (
		opcode == Opcode::Continue ||
		opcode == Opcode::Binary ||
		opcode == Opcode::Text ||
		opcode == Opcode::Ping ||
		opcode == Opcode::Pong ||
		opcode == Opcode::Close
	);
}

struct IncomingFrame : public Frame {
	size_t next_seek = 0;
	size_t payload_len = 0;
	std::optional<Ws::FrameMask> mask;
};

std::optional<IncomingFrame> parse_frame(const std::vector<uint8_t>& read_buff) {

	size_t next_seek = 2;
	if (read_buff.size() < next_seek) {
		return std::nullopt;
	}

	auto frame_bit = static_cast<FrameBit>(read_buff[0] & 0xF0);
	if (!is_valid_frame_bit(frame_bit)) {
		throw std::runtime_error("Received invalid frame bit");
	}

	auto op_code = static_cast<Opcode>(read_buff[0] & 0x0F);
	if (!is_valid_opcode(op_code)) {
		throw std::runtime_error("Received invalid opcode");
	}

	bool has_mask = (read_buff[1] & 0x80) >> 7;

	auto payload_len = unpack_paylen(read_buff, next_seek);
	if (!payload_len.has_value()) {
		return std::nullopt;
	}

	auto mask = has_mask ? unpack_mask(read_buff, next_seek) : std::nullopt;
	if (has_mask && !mask.has_value()) {
		return std::nullopt;
	}

	return IncomingFrame {
		Frame {
			.frame = frame_bit,
			.code = op_code,
		},
		next_seek,
		payload_len.value(),
		mask
	};
}

size_t Websocket::write(const std::string& message) {
	return this->write(Frame {
		.frame = FrameBit::Final,
		.code = Opcode::Text,
		.data = std::vector<uint8_t>(message.begin(), message.end())
	});
}

size_t Websocket::write(const std::vector<uint8_t>& data) {
	return this->write(Frame {
		.frame = FrameBit::Final,
		.code = Opcode::Binary,
		.data = data
	});
}

size_t Websocket::write(const Frame& frame) {

	if (this->m_closed) {
		throw std::runtime_error("Websocket: Cannot write to a closed socket");
	}

	uint8_t frame_bit = static_cast<uint8_t>(frame.frame);
	uint8_t op_code = static_cast<uint8_t>(frame.code);

	std::vector<uint8_t> buff {
		static_cast<uint8_t>(frame_bit | op_code)
	};

	auto size_buff = pack_paylen(frame.data.size());

	buff.insert(buff.end(), size_buff.begin(), size_buff.end());
	buff.insert(buff.end(), frame.data.begin(), frame.data.end());

	return this->m_writer.write(buff);
}

std::optional<Frame> Websocket::next() {

	if (this->m_closed) {
		throw std::runtime_error("Websocket: Cannot read messages from a closed socket");
	}

	auto frame_opt = parse_frame(this->m_read_buff);
	if (!frame_opt.has_value()) {

		if (this->m_read_buff.size() > max_header_size) {
			throw std::runtime_error("Read buffer clogging detected: Data w/o frame headers");
		}

		auto chunk = this->m_reader.read(read_chunk_size);
		if (chunk.empty()) {
			return std::nullopt;
		}

		this->m_read_buff.insert(this->m_read_buff.end(), chunk.begin(), chunk.end());

		frame_opt = parse_frame(this->m_read_buff);
		if (!frame_opt.has_value()) {
			return std::nullopt;
		}
	}

	auto frame = frame_opt.value();
	auto frame_end = frame.next_seek + frame.payload_len;

	if (this->m_read_buff.size() >= frame_end) {

		auto buff = std::vector<uint8_t>(this->m_read_buff.begin() + frame.next_seek, this->m_read_buff.begin() + frame_end);

		this->m_read_buff.erase(this->m_read_buff.begin(), this->m_read_buff.begin() + frame_end);

		frame.data = buff;

	} else {

		auto unbuffered_len = frame.payload_len - (this->m_read_buff.size() - frame.next_seek);
		auto chunk = this->m_reader.read(unbuffered_len);

		if (chunk.size() < unbuffered_len) {
			this->m_read_buff.insert(this->m_read_buff.end(), chunk.begin(), chunk.end());
			return std::nullopt;
		}

		frame.data.insert(frame.data.end(), this->m_read_buff.begin() + frame.next_seek, this->m_read_buff.end());
		frame.data.insert(frame.data.end(), chunk.begin(), chunk.end());

		this->m_read_buff.clear();
	}

	switch (frame.frame) {

		case FrameBit::Continue: {

			if (!this->m_multipart.has_value()) {
				this->m_multipart = MultipartState {
					.code = frame.code,
					.mask = frame.mask
				};
			} else {
				frame.mask = this->m_multipart.value().mask;
			}

		} break;

		case FrameBit::Final: {

			if (this->m_multipart.has_value()) {
				this->m_multipart.reset();
			}

		} break;
	
		default: break;
	}

	//	unmask the payload
	if (frame.mask.has_value()) {
		const auto& mask = frame.mask.value();
		for (size_t i = 0; i < frame.data.size(); i++) {
			frame.data[i] ^= mask[i % mask.size()];
		}
	}

	return frame;
}

size_t Websocket::close(Ws::CloseReason reason) {

	auto reason_code = static_cast<std::underlying_type_t<CloseReason>>(reason);

	std::vector<uint8_t> buff ({
		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			static_cast<uint8_t>((reason_code >> 8) & 0xff),
			static_cast<uint8_t>(reason_code & 0xff),
		#else
			static_cast<uint8_t>(reason_code & 0xff),
			static_cast<uint8_t>((reason_code >> 8) & 0xff),
		#endif
	});

	Frame frame {
		.code = Opcode::Close,
		.data = buff
	};

	size_t written =  this->write(frame);
	if (written > 0) {
		this->m_closed = true;
	}

	return written;
}

bool Websocket::is_open() const noexcept {
	return this->m_reader.is_readable() || this->m_writer.writable();
}
