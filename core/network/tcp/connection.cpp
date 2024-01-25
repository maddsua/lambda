#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

Connection::Connection(ConnCreateInit init) {

	this->hSocket = init.hSocket;
	this->info = init.info;

	auto connTimeoutValue = init.info.timeout > 0 ? init.info.timeout : Connection::TimeoutMs_Default;

	try {
		this->setTimeouts(connTimeoutValue);
	} catch(const std::exception& err) {
		if (this->hSocket != INVALID_SOCKET)
			closesocket(this->hSocket);
		throw err;
	}
}

Connection& Connection::operator= (Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->info = other.info;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

Connection::Connection(Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->info = other.info;
	other.hSocket = INVALID_SOCKET;
}

Connection::~Connection() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

void Connection::closeconn() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
	this->hSocket = INVALID_SOCKET;
}

const ConnectionInfo& Connection::getInfo() const noexcept {
	return this->info;
}

bool Connection::ok() const noexcept {
	return this->hSocket != INVALID_SOCKET;
}

void Connection::write(const std::vector<uint8_t>& data) {

	if (this->hSocket == INVALID_SOCKET)
		throw std::runtime_error("can't write to a closed connection");

	std::lock_guard<std::mutex> lock(this->writeMutex);

	auto bytesSent = send(this->hSocket, (const char*)data.data(), data.size(), 0);

	if (static_cast<size_t>(bytesSent) != data.size()) {
		this->hSocket = INVALID_SOCKET;
		throw std::runtime_error("write failed: network error " + std::to_string(getAPIError()));
	}
}

std::vector<uint8_t> Connection::read() {
	return this->read(this->ReadChunkSize);
}

std::vector<uint8_t> Connection::read(size_t expectedSize) {

	if (this->hSocket == INVALID_SOCKET)
		throw std::runtime_error("can't read from a closed connection");

	std::lock_guard<std::mutex> lock(this->readMutex);

	std::vector<uint8_t> chunk;
	chunk.resize(expectedSize);

	auto bytesReceived = recv(this->hSocket, (char*)chunk.data(), chunk.size(), 0);

	if (bytesReceived == 0) {

		this->closeconn();
		return {};

	} else if (bytesReceived < 0) {

		auto apiError = getAPIError();

		switch (apiError) {

			case LNE_TIMEDOUT: {
				this->closeconn();
				return {};
			} break;
			
			default: {
				this->hSocket = INVALID_SOCKET;
				throw std::runtime_error("read failed: network error " + std::to_string(apiError));
			}
		}
	}

	chunk.resize(bytesReceived);
	chunk.shrink_to_fit();

	return chunk;
}

void Connection::setTimeouts(uint32_t value) {
	this->setTimeout(value, SetConnectionTimeoutDirection::Both);
}

void Connection::setTimeout(uint32_t value, SetConnectionTimeoutDirection direction) {

	if (direction != SetConnectionTimeoutDirection::Tx) {
		if (setsockopt(this->hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&value, sizeof(value)))
			throw std::runtime_error("failed to set socket RX timeout: code " + std::to_string(getAPIError()));
	}

	if (direction != SetConnectionTimeoutDirection::Rx) {
		if (setsockopt(this->hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&value, sizeof(value)))
			throw std::runtime_error("failed to set socket TX timeout: code " + std::to_string(getAPIError()));
	}
}
