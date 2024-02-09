#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

Connection::Connection(
	SockHandle handleInit,
	const ConnectionInfo& infoInit
) : hSocket(handleInit), m_info(infoInit) {}

Connection& Connection::operator= (Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->m_info = other.m_info;
	other.hSocket = INVALID_SOCKET;
	return *this;
}

Connection::Connection(Connection&& other) noexcept {
	this->hSocket = other.hSocket;
	this->m_info = other.m_info;
	other.hSocket = INVALID_SOCKET;
}

Connection::~Connection() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
}

void Connection::end() noexcept {

	if (this->hSocket == INVALID_SOCKET) return;

	//	swapping handle to a temp variable so that
	//	no race condition can occur further down the chain
	auto tempHandle = this->hSocket;
	this->hSocket = INVALID_SOCKET;

	shutdown(tempHandle, SD_BOTH);
	closesocket(tempHandle);
}

const ConnectionInfo& Connection::info() const noexcept {
	return this->m_info;
}

bool Connection::active() const noexcept {
	return this->hSocket != INVALID_SOCKET;
}

void Connection::write(const std::vector<uint8_t>& data) {

	if (this->hSocket == INVALID_SOCKET)
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

	if (this->hSocket == INVALID_SOCKET)
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

void Connection::setTimeouts(uint32_t valueMs, SetTimeoutsDirection direction) {

	#ifdef _WIN32
		const auto timeoutValue = valueMs;
	#else
		timeval timeoutValue;
		timeoutValue.tv_sec = value / 1000;
		timeoutValue.tv_usec = 0;
	#endif

	const auto timeouValuePtr = reinterpret_cast<const char*>(&timeoutValue);

	if (direction != SetTimeoutsDirection::Receive) {
		if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, timeouValuePtr, sizeof(timeoutValue)))
			throw Lambda::APIError("failed to set socket TX timeout");
		this->m_info.timeouts.send = valueMs;
	}

	if (direction != SetTimeoutsDirection::Send) {
		if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, timeouValuePtr, sizeof(timeoutValue)))
			throw Lambda::APIError("failed to set socket RX timeout");
		this->m_info.timeouts.receive = valueMs;
	}
}

void Connection::setTimeouts(uint32_t value) {
	this->setTimeouts(value, SetTimeoutsDirection::Both);	
}
