#include <ctime>
#include <array>

#include "../../network/tcp/connection.hpp"
#include "../../websocket/message.hpp"
#include "../../websocket/proto.hpp"
#include "../websocket.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

WebsocketStream::WebsocketStream() {

}

WebsocketStream::~WebsocketStream() {
	if (this->ioworker.valid()) {
		try { this->ioworker.get(); } catch(...) {}
	}
}

bool WebsocketStream::available() const noexcept {
	return this->rxQueue.size();
}

bool WebsocketStream::ok() const noexcept {
	if (this->conn == nullptr) return false;
	return this->conn->ok();
}

Message WebsocketStream::getMessage() {

	if (!this->available()) throw std::runtime_error("no messages available");

	std::lock_guard<std::mutex> lock(this->writeMutex);

	Message temp = this->rxQueue.front();
	this->rxQueue.pop();
	return temp;
}

void WebsocketStream::sendMessage(const Message& msg) {
	auto messageBuffer = serializeMessage(msg);
	std::lock_guard<std::mutex> lock(this->writeMutex);
	this->txQueue.insert(this->txQueue.end(), messageBuffer.begin(), messageBuffer.end());
}

void WebsocketStream::close() {
	this->close(CloseCode::Normal);
}

void WebsocketStream::close(CloseCode reason) {

	this->terminateFlags = StreamTerminateFlags::Closed;

	std::array<uint8_t, 4> closeFrame;

	auto closeReasonByte = static_cast<std::underlying_type_t<CloseCode>>(reason);

	//	control frame and close opcode
	closeFrame[0] = 0x88;
	//	should always be 2 bytes, we only send a status code with no text
	closeFrame[1] = sizeof(closeReasonByte);
	//	the status code itself. I hate the ppl who decided not to align ws header fields. just effing masterminds.
	closeFrame[2] = (closeReasonByte >> 8) & 0xFF;
	closeFrame[3] = closeReasonByte & 0xFF;

	//	send and forget it
	std::lock_guard<std::mutex> lock(this->writeMutex);
	this->txQueue.insert(this->txQueue.end(), closeFrame.begin(), closeFrame.end());

	if (this->ioworker.valid()) {
		this->ioworker.get();
	}
}

void WebsocketStream::terminate() {
	this->terminateFlags = StreamTerminateFlags::Terminated;
	if (this->ioworker.valid()) {
		this->ioworker.get();
	}
}
