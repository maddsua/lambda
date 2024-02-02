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
		init.connbuff.clear();
		this->conn.setTimeouts(100, Network::SetTimeoutsDirection::Receive);

		while (this->conn.active() && !this->m_stopped) {

			auto next = this->conn.read();
			if (!next.size()) continue;

			downloadBuff.insert(downloadBuff.end(), next.begin(), next.end());
			if (downloadBuff.size() < 2) continue;

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
