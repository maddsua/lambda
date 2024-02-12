#include "./websocket.hpp"
#include "./transport.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Websocket::Transport;

static const std::string wsPingString = "ping/lambda/ws";

//	these values are used for both pings and actual receive timeouts
static const time_t wsActTimeout = 5000;
static const unsigned short wsMaxSkippedAttempts = 3;
static const size_t wsReadChunk = 256;

static const std::initializer_list<OpCode> supportedWsOpcodes = {
	OpCode::Binary,
	OpCode::Text,
	OpCode::Continue,
	OpCode::Close,
	OpCode::Ping,
	OpCode::Pong,
};

void WebsocketContext::sendMessage(const Websocket::Message& msg) {
	auto writeBuff = serializeMessage(msg);
	this->transport.writeRaw(writeBuff);
}

FrameHeader Transport::parseFrameHeader(const std::vector<uint8_t>& buffer) {

	FrameHeader header {
		static_cast<FrameControlBits>(buffer.at(0) & 0xF0),
		static_cast<OpCode>(buffer.at(0) & 0x0F),
		static_cast<size_t>(2),
		static_cast<size_t>(buffer.at(1) & 0x7F)
	};

	if (header.payloadSize == 126) {
		header.size += 2;
		header.payloadSize = (buffer.at(2) << 8) | buffer.at(3);
	} else if (header.payloadSize == 127) {
		header.size += 8;
		header.payloadSize = 0;
		for (int i = 0; i < 8; i++) {
			header.payloadSize |= (buffer.at(2 + i) << ((7 - i) * 8));
		}
	}

	bool maskUsed = (buffer.at(1) & 0x80) >> 7;
	if (maskUsed) {

		if (buffer.size() < header.size + FrameHeader::mask_size) {
			throw std::runtime_error("invalid websocket frame: not enough data to read mask");
		}

		std::array<uint8_t, FrameHeader::mask_size> mask;

		for (size_t i = 0; i < mask.size(); i++) {
			mask[i] = buffer.at(header.size + i);
		}

		header.mask = mask;
		header.size += 4;
	}

	return header;
}

std::vector <uint8_t> Transport::serializeFrameHeader(const FrameHeader& header) {

	std::vector<uint8_t> resultBuffer;

	uint8_t finBit = static_cast<std::underlying_type_t<FrameControlBits>>(header.finbit);
	uint8_t opCode = static_cast<std::underlying_type_t<OpCode>>(header.opcode);

	resultBuffer.push_back(finBit | opCode);

	if (header.payloadSize < 126) {
		resultBuffer.push_back(header.payloadSize & 0x7F);
	} else if (header.payloadSize >= 126 && header.payloadSize <= 65535) {
		resultBuffer.push_back(126);
		resultBuffer.push_back((header.payloadSize >> 8) & 255);
		resultBuffer.push_back(header.payloadSize & 255);
	} else {
		resultBuffer.push_back(127);
		for (int i = 0; i < 8; i++) {
			resultBuffer.push_back((header.payloadSize >> ((7 - i) * 8)) & 0xFF);
		}
	}

	return resultBuffer;
}

std::vector<uint8_t> Transport::serializeMessage(const Message& message) {

	//	create frame buffer
	FrameHeader header {
		message.partial ? FrameControlBits::BitContinue : FrameControlBits::BitFinal,
		message.binary ? OpCode::Binary : OpCode::Text,
		static_cast<size_t>(0),
		message.size(),
	};

	auto resultBuffer = serializeFrameHeader(header);
	resultBuffer.insert(resultBuffer.end(), message.data.begin(), message.data.end());

	return resultBuffer;
}

void WebsocketContext::asyncWorker() {

	std::vector<uint8_t> downloadBuff;
	std::optional<MultipartMessageContext> multipartCtx;

	auto lastPing = std::chrono::steady_clock::now();
	auto lastPingResponse = std::chrono::steady_clock::now();
	const auto pingWindow = std::chrono::milliseconds(wsMaxSkippedAttempts * wsActTimeout);

	while (this->transport.isConnected() && !this->m_stopped) {

		//	send ping or terminate websocket if there is no response
		if ((lastPing - lastPingResponse) > pingWindow) {
			this->close(CloseReason::ProtocolError);
			throw std::runtime_error("Didn't receive any response for pings");

		} else if ((std::chrono::steady_clock::now() - lastPing) > std::chrono::milliseconds(wsActTimeout)) {

			auto pingHeader = serializeFrameHeader({
				FrameControlBits::BitFinal,
				OpCode::Ping,
				static_cast<size_t>(0),
				wsPingString.size()
			});

			this->transport.writeRaw(pingHeader);
			this->transport.writeRaw(std::vector<uint8_t>(wsPingString.begin(), wsPingString.end()));

			lastPing = std::chrono::steady_clock::now();
		}

		auto nextChunk = this->transport.readRaw(wsReadChunk);
		if (!nextChunk.size()) continue;

		downloadBuff.insert(downloadBuff.end(), nextChunk.begin(), nextChunk.end());
		if (downloadBuff.size() < FrameHeader::min_size) continue;

		if (downloadBuff.size() > this->topts.maxRequestSize) {
			this->close(CloseReason::MessageTooBig);
			throw std::runtime_error("Expected frame size too large");
		}

		auto frameHeader = parseFrameHeader(downloadBuff);

		bool opcodeValid = false;
		for (const auto& item : supportedWsOpcodes) {
			if (frameHeader.opcode != item) continue;
			opcodeValid = true;
			break;
		}

		if (!opcodeValid) {
			auto opcodeInt = static_cast<std::underlying_type_t<OpCode>>(frameHeader.opcode);
			this->close(CloseReason::ProtocolError);
			throw std::runtime_error("received an invalid opcode (" + std::to_string(opcodeInt) + ")");
		}

		auto frameSize = frameHeader.size + frameHeader.payloadSize;
		auto payloadBuff = std::vector<uint8_t>(downloadBuff.begin() + frameHeader.size, downloadBuff.begin() + frameSize);

		if (frameSize > this->topts.maxRequestSize) {
			this->close(CloseReason::MessageTooBig);
			throw std::runtime_error("frame size too large");
		}

		if (frameHeader.payloadSize + frameHeader.payloadSize < downloadBuff.size()) {

			auto expectedSize = frameHeader.payloadSize - payloadBuff.size();
			auto payloadChunk = this->transport.readRaw(expectedSize);

			if (payloadChunk.size() < expectedSize) {
				this->close(CloseReason::ProtocolError);
				throw std::runtime_error("failed to read websocket frame: not enough data");
			}

			payloadBuff.insert(payloadBuff.end(), payloadChunk.begin(), payloadChunk.end());
		}

		if (!(frameHeader.mask.has_value() || multipartCtx.has_value())) {
			this->close(CloseReason::ProtocolError);
			throw std::runtime_error("received unmasked data from the client");
		}

		if (frameHeader.finbit != FrameControlBits::BitFinal) {

			if (!multipartCtx.has_value()) {

				multipartCtx = MultipartMessageContext({
					frameHeader.mask.value(),
					frameHeader.opcode == OpCode::Binary
				});

			} else {
				frameHeader.mask = multipartCtx.value().mask;
			}
		}

		//	unmask the payload
		auto& frameMask = frameHeader.mask.value();
		for (size_t i = 0; i < payloadBuff.size(); i++) {
			payloadBuff[i] ^= frameMask[i % 4];
		}

		switch (frameHeader.opcode) {

			case OpCode::Close: {
				this->m_stopped = true;
				this->close(CloseReason::Normal);
			} break;

			case OpCode::Ping: {

				auto pongHeader = serializeFrameHeader({
					FrameControlBits::BitFinal,
					OpCode::Pong,
					static_cast<size_t>(0),
					frameHeader.payloadSize
				});

				this->transport.writeRaw(pongHeader);
				this->transport.writeRaw(payloadBuff);

			} break;

			case OpCode::Pong: {

				//	check that pong payload matches the ping's one
				if (std::equal(payloadBuff.begin(), payloadBuff.end(), wsPingString.begin(), wsPingString.end())) {
					lastPingResponse = std::chrono::steady_clock::now();
				} else {
					this->close(CloseReason::ProtocolError);
					throw std::runtime_error("invalid pong reponse");
				}

			} break;

			default: {

				auto isBinary = frameHeader.opcode == OpCode::Continue ?
					multipartCtx.value().binary : 
					frameHeader.opcode == OpCode::Binary;

				std::lock_guard<std::mutex>lock(this->m_read_lock);
				this->m_queue.push({
					payloadBuff,
					isBinary,
					frameHeader.finbit == FrameControlBits::BitFinal
				});
			}
		}

		if (frameHeader.finbit == FrameControlBits::BitFinal && multipartCtx.has_value()) {
			multipartCtx = std::nullopt;
		}

		downloadBuff.erase(downloadBuff.begin(), downloadBuff.begin() + frameHeader.size + frameHeader.payloadSize);
	}
}
