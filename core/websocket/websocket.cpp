#include "./websocket.hpp"
#include "./transport.hpp"
#include "../http/transport.hpp"
#include "../server/server_impl.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../utils/utils.hpp"

#include <cstring>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;
using namespace Lambda::Websocket;
using namespace Lambda::Websocket::Transport;
using namespace Lambda::Server::Connection;

static const std::string wsMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//	The recv function blocks execution infinitely until it receives somethig,
//	which is not optimal for this usecase.
//	There's an overlapped io in winapi, but it's kinda ass and definitely is not portable
//	So I'm reinventing a wheel by causing receive function to fail quite often
//	so that at enabled the receive loop to be terminated at any time
//	It works, so fuck that, I'm not even selling this code to anyone. Yet. Remove when you do, the future Daniel.
static const time_t sockRcvTimeout = 100;

WebsocketContext::WebsocketContext(WebsocketInit init) :
	m_worker(init.workerctx), m_transport(init.transport), m_topts(init.transport.options()) {

	const auto& request = init.requestEvent.request;

	if (request.method != "GET") {
		throw UpgradeError("Websocket handshake method invalid", 405);
	}

	auto headerUpgrade = Strings::toLowerCase(request.headers.get("Upgrade"));
	auto headerWsKey = request.headers.get("Sec-WebSocket-Key");

	if (headerUpgrade != "websocket" || !headerWsKey.size()) {
		throw UpgradeError("Websocket handshake header invalid", 400);
	}

	if (this->m_transport.hasPartialData()) {
		throw UpgradeError("Websocket handshake has extra data after headers", 400);
	}

	const auto combinedKey = headerWsKey + wsMagicString;
	const auto keyHash = Crypto::SHA1().update(combinedKey).digest();

	auto handshakeReponse = HTTP::Response(101, {
		{ "Upgrade", "websocket" },
		{ "Connection", "Upgrade" },
		{ "Sec-WebSocket-Accept", Encoding::toBase64(keyHash) }
	});

	this->m_transport.respond({ handshakeReponse, init.requestEvent.id });
	this->m_transport.reset();

	this->m_transport.tcpconn().flags.closeOnTimeout = false;
	this->m_transport.tcpconn().setTimeouts(sockRcvTimeout, Network::SetTimeoutsDirection::Receive);

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

	this->m_transport.writeRaw(closeMessageBuff);
	this->m_transport.close();
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

	if (!this->m_reader.valid() || this->m_worker.finished) {
		return this->m_queue.size();
	}

	auto readerDone = false;
	while (!readerDone && !this->m_queue.size() && !this->m_worker.finished) {
		readerDone = this->m_reader.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
	}

	if (readerDone) {
		this->m_reader.get();
	}

	return this->m_queue.size();
}


Message::Message(const std::string& dataInit) :
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	binary(false),
	partial(false),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::string& dataInit, bool partialInit) :
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	binary(false),
	partial(partialInit),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit) :
	data(dataInit),
	binary(true),
	partial(false),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit, bool partialInit) :
	data(dataInit),
	binary(true),
	partial(partialInit),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit, bool binaryInit, bool partialInit) :
	data(dataInit),
	binary(binaryInit),
	partial(partialInit),
	timestamp(std::time(nullptr))
{}

std::string Message::text() const noexcept {
	return std::string(this->data.begin(), this->data.end());
}

size_t Message::size() const noexcept {
	return this->data.size();
}
