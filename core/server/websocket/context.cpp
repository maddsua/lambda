#include "../server.hpp"
#include "../internal.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Server::WSTransport;

static const std::string wsPingString = "ping/lambda/ws";

/*static const std::array<uint8_t, 6> wsOpCodes = {
	static_cast<std::underlying_type_t<CloseReason>>(reason)
	OpCode::Binary,
	OpCode::Text,
	OpCode::Cont,
	OpCode::Close,
	OpCode::Ping,
	OpCode::Pong,
};*/

WebsocketContext::WebsocketContext(ContextInit init) : conn(init.conn) {

	this->conn.flags.closeOnTimeout = false;

	this->m_reader = std::async([&]() {

		//	I should probably call move here
		std::vector<uint8_t> downloadBuff = init.connbuff;
		std::optional<std::array<uint8_t, WebsocketFrameHeader::mask_size>> multipartDataMask;
		init.connbuff.clear();
		this->conn.setTimeouts(100, Network::SetTimeoutsDirection::Receive);

		while (this->conn.active() && !this->m_stopped) {

			auto nextChunk = this->conn.read();
			if (!nextChunk.size()) continue;

			downloadBuff.insert(downloadBuff.end(), nextChunk.begin(), nextChunk.end());
			if (downloadBuff.size() < WebsocketFrameHeader::min_size) continue;

			auto frameHeader = parseFrameHeader(downloadBuff);

			//	check opcode
			/*bool opcodeSupported = std::any_of(wsOpCodes.begin(), wsOpCodes.end(), [&frameHeader](auto element) {
				return element == frameHeader.opcode;
			});*/

			auto frameSize = frameHeader.size + frameHeader.payloadSize;
			auto payloadBuff = std::vector<uint8_t>(downloadBuff.begin() + frameHeader.size, downloadBuff.begin() + frameSize);

			if (frameHeader.payloadSize + frameHeader.payloadSize < downloadBuff.size()) {

				auto expectedSize = frameHeader.payloadSize - payloadBuff.size();
				auto payloadChunk = this->conn.read(expectedSize);

				if (payloadChunk.size() < expectedSize) {
					throw std::runtime_error("failed to read websocket frame: not enough data");
				}

				payloadBuff.insert(payloadBuff.end(), payloadChunk.begin(), payloadChunk.end());
			}

			//	unmask the payload
			if (frameHeader.finbit != WebsockBits::BitFinal) {
				if (frameHeader.mask.has_value()) {
					multipartDataMask = frameHeader.mask;
				} else {
					frameHeader.mask = multipartDataMask;
				}
			}

			if (!frameHeader.mask.has_value()) {
				throw std::runtime_error("received unmasked data from the client");
			}

			auto& frameMask = frameHeader.mask.value();
			for (size_t i = 0; i < payloadBuff.size(); i++) {
				payloadBuff[i] ^= frameMask[i % 4];
			}
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

	auto closeReasonCode = static_cast<std::underlying_type_t<CloseReason>>(reason);

	auto closeMessageBuff = serializeFrameHeader({
		WebsockBits::BitFinal,
		OpCode::Close,
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
