#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

Connection::Connection(const ConnInit& init) {
	this->m_info = init.info;
	this->hSocket = init.hSocket;
}

Connection& Connection::operator= (Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->m_info = other.m_info;
	other.hSocket = Network::invalid_socket;
	return *this;
}

Connection::Connection(Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->m_info = other.m_info;
	other.hSocket = Network::invalid_socket;
}

Connection::~Connection() {
	if (this->hSocket == Network::invalid_socket) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

void Connection::end() noexcept {

	if (this->hSocket == Network::invalid_socket) return;

	//	swapping handle to a temp variable so that
	//	no race condition can occur further down the chain
	auto tempHandle = this->hSocket;
	this->hSocket = Network::invalid_socket;

	shutdown(tempHandle, SD_BOTH);
	closesocket(tempHandle);
}

const ConnectionInfo& Connection::info() const noexcept {
	return this->m_info;
}

bool Connection::active() const noexcept {
	return this->hSocket != Network::invalid_socket;
}

void Connection::write(const std::vector<uint8_t>& data) {

	if (this->hSocket == Network::invalid_socket)
		throw std::runtime_error("cann't write to a closed connection");

	std::lock_guard<std::mutex> lock(this->m_writeMutex);

	auto bytesSent = send(this->hSocket, (const char*)data.data(), data.size(), 0);

	if (static_cast<size_t>(bytesSent) != data.size())
		throw Lambda::APIError("network error while sending data");
}

std::vector<uint8_t> Connection::read() {
	return this->read(this->ReadChunkSize);
}

std::vector<uint8_t> Connection::read(size_t expectedSize) {

	if (this->hSocket == Network::invalid_socket)
		throw std::runtime_error("can't read from a closed connection");

	std::lock_guard<std::mutex> lock(this->m_readMutex);

	std::vector<uint8_t> chunk;
	chunk.resize(expectedSize);

	auto bytesReceived = recv(this->hSocket, (char*)chunk.data(), chunk.size(), 0);

	if (bytesReceived == 0) {

		this->end();
		return {};

	} else if (bytesReceived < 0) {

		auto apiError = Errors::getApiError();

		switch (apiError) {

			case LNE_TIMEDOUT: {

				if (this->flags.closeOnTimeout) {
					this->end();
				}

				return {};
			}

			default:
				throw Lambda::APIError(apiError, "network error while receiving data");
		}	
	}

	chunk.resize(bytesReceived);
	chunk.shrink_to_fit();

	return chunk;
}

void Connection::setTimeouts(uint32_t value, SetTimeoutsDirection direction) {

	if (direction != SetTimeoutsDirection::Receive) {
		if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&value, sizeof(value)))
			throw Lambda::APIError("failed to set socket TX timeout");
		this->m_info.timeouts.send = value;
	}

	if (direction != SetTimeoutsDirection::Send) {
		if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&value, sizeof(value)))
			throw Lambda::APIError("failed to set socket RX timeout");
		this->m_info.timeouts.receive = value;
	}
}

void Connection::setTimeouts(uint32_t value) {
	this->setTimeouts(value, SetTimeoutsDirection::Both);	
}
