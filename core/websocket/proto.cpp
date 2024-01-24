#include <ctime>

#include "../network/tcp/connection.hpp"
#include "./internal.hpp"
#include "./websocket.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

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
	std::lock_guard<std::mutex> lock(this->writeMutex);
	this->txQueue.push(msg);
}

void WebsocketStream::close() {
	this->close(CloseCode::Normal);
}

void WebsocketStream::close(CloseCode reason) {

}
