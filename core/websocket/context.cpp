#include "./websocket.hpp"
#include "./transport.hpp"
#include "../http/transport.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../utils/utils.hpp"

#include <cstring>

using namespace Lambda;
using namespace Lambda::Websocket;
using namespace Lambda::Websocket::Transport;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//	The recv function blocks execution infinitely until it receives somethig,
//	which is not optimal for this usecase.
//	There's an overlapped io in winapi, but it's kinda ass and definitely is not portable
//	So I'm reinventing a wheel by causing receive function to fail quite often
//	so that at enabled the receive loop to be terminated at any time
//	It works, so fuck that, I'm not even selling this code to anyone. Yet. Remove when you do, the future Daniel.
static const time_t sockRcvTimeout = 100;

WebsocketContext::WebsocketContext(HTTP::Transport::TransportContextV1& httpCtx, const HTTP::Request initRequest)
	: conn(httpCtx.tcpconn()), topts(httpCtx.options()) {

	auto headerUpgrade = Strings::toLowerCase(initRequest.headers.get("Upgrade"));
	auto headerWsKey = initRequest.headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size()) {
		throw std::runtime_error("Websocket initialization aborted: Invalid connection header");
	}

	if (httpCtx.hasPartialData()) {
		throw std::runtime_error("Websocket initialization aborted: Connection has unprocessed data");
	}

	auto combinedKey = headerWsKey + wsMagicString;

	auto keyHash = Crypto::SHA1().update(combinedKey).digest();

	auto handshakeReponse = HTTP::Response(101, {
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::toBase64(keyHash) }
	});

	httpCtx.respond(handshakeReponse);
	httpCtx.reset();

	this->conn.flags.closeOnTimeout = false;
	this->conn.setTimeouts(sockRcvTimeout, Network::SetTimeoutsDirection::Receive);

	this->m_reader = std::async(&WebsocketContext::asyncWorker, this);
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
