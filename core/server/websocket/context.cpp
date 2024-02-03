#include "../server.hpp"
#include "../internal.hpp"
#include "../../utils/utils.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Server::WSTransport;

static const std::string wsPingString = "ping/lambda/ws";

//	The recv function blocks execution infinitely until it receives somethig,
//	which is not optimal for this usecase.
//	There's an overlapped io in winapi, but it's kinda ass and definitely is not portable
//	So I'm reinventing a wheel by causing receive function to fail quite often
//	so that at enabled the receive loop to be terminated at any time
//	It works, so fuck that, I'm not even selling this code to anyone. Yet. Remove when you do, the future Daniel.
static const time_t wsRcvTimeout = 100;

//	these values are used for both pings and actual receive timeouts
static const time_t wsActTimeout = 5000;
static const unsigned short wsMaxSkippedAttempts = 3;

static const std::initializer_list<OpCode> supportedWsOpcodes = {
	OpCode::Binary,
	OpCode::Text,
	OpCode::Continue,
	OpCode::Close,
	OpCode::Ping,
	OpCode::Pong,
};

WebsocketContext::WebsocketContext(Network::TCP::Connection& connRef) : conn(connRef) {

	this->conn.flags.closeOnTimeout = false;
	this->conn.setTimeouts(wsRcvTimeout, Network::SetTimeoutsDirection::Receive);

	this->m_reader = std::async([&]() {

		std::vector<uint8_t> downloadBuff;
		std::optional<MultipartMessageContext> multipartCtx;

		auto lastPing = std::chrono::steady_clock::now();
		auto lastPingResponse = std::chrono::steady_clock::now();
		auto pingWindow = std::chrono::milliseconds(wsMaxSkippedAttempts * wsActTimeout);

		while (this->conn.active() && !this->m_stopped) {

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

				this->conn.write(pingHeader);
				this->conn.write(std::vector<uint8_t>(wsPingString.begin(), wsPingString.end()));

				lastPing = std::chrono::steady_clock::now();
			}

			auto nextChunk = this->conn.read();
			if (!nextChunk.size()) continue;

			downloadBuff.insert(downloadBuff.end(), nextChunk.begin(), nextChunk.end());
			if (downloadBuff.size() < FrameHeader::min_size) continue;

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

			if (frameHeader.payloadSize + frameHeader.payloadSize < downloadBuff.size()) {

				auto expectedSize = frameHeader.payloadSize - payloadBuff.size();
				auto payloadChunk = this->conn.read(expectedSize);

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

					this->conn.write(pongHeader);
					this->conn.write(payloadBuff);

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
	});
}

WebsocketContext::~WebsocketContext() {

	this->m_stopped = true;

	if (this->m_reader.valid()) {
		try { this->m_reader.get(); }
			catch (...) {}
	}
}

void WebsocketContext::close(Websocket::CloseReason reason) {

	this->m_stopped = true;

	auto closeReasonCode = Bits::netwnormx(static_cast<std::underlying_type_t<CloseReason>>(reason));

	auto closeMessageBuff = serializeFrameHeader({
		FrameControlBits::BitFinal,
		OpCode::Close,
		static_cast<size_t>(0),
		sizeof(closeReasonCode)
	});

	std::array<uint8_t, sizeof(closeReasonCode)> closeReasonBuff;
	memcpy(closeReasonBuff.data(), &closeReasonCode, sizeof(closeReasonCode));

	closeMessageBuff.insert(closeMessageBuff.end(), closeReasonBuff.begin(), closeReasonBuff.end());	

	this->conn.write(closeMessageBuff);

	if (this->m_reader.valid()) {
		try { this->m_reader.get(); }
			catch (...) {}
	}
}

bool WebsocketContext::hasMessage() const noexcept {
	return this->m_queue.size() > 0;
}

Message WebsocketContext::nextMessage() {

	if (!this->m_queue.size()) {
		throw std::runtime_error("cannot get next item from an empty HttpRequestQueue");
	}

	std::lock_guard<std::mutex>lock(this->m_read_lock);

	Message temp = this->m_queue.front();
	this->m_queue.pop();

	return temp;
}

bool WebsocketContext::awaitMessage() {

	if (!this->m_reader.valid()) {
		return this->m_queue.size();
	}

	auto readerDone = false;
	while (!readerDone && !this->m_queue.size()) {
		readerDone = this->m_reader.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
	}

	if (readerDone) {
		this->m_reader.get();
	}

	return this->m_queue.size();
}
