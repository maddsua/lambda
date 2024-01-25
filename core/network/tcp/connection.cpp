#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

Connection::Connection(ConnCreateInit init) {
	this->m_info = init.info;
	this->m_socket = init.hSocket;
}

Connection& Connection::operator= (Connection&& other) noexcept {
	this->m_socket = other.m_socket;
	this->m_info = other.m_info;
	other.m_socket = INVALID_SOCKET;
	return *this;
}

Connection::Connection(Connection&& other) noexcept {
	this->m_socket = other.m_socket;
	this->m_info = other.m_info;
	other.m_socket = INVALID_SOCKET;
}

Connection::~Connection() {
	if (this->m_socket == INVALID_SOCKET) return;
	shutdown(this->m_socket, SD_BOTH);
	closesocket(this->m_socket);
}

void Connection::end() {
	if (this->m_socket == INVALID_SOCKET) return;
	shutdown(this->m_socket, SD_BOTH);
	closesocket(this->m_socket);
	this->m_socket = INVALID_SOCKET;
}

const ConnectionInfo& Connection::info() const noexcept {
	return this->m_info;
}

bool Connection::isOpen() const noexcept {
	return this->m_socket != INVALID_SOCKET;
}

void Connection::write(const std::vector<uint8_t>& data) {

	if (this->m_socket == INVALID_SOCKET)
		throw std::runtime_error("cann't write to a closed connection");

	std::lock_guard<std::mutex> lock(this->m_writeMutex);

	auto bytesSent = send(this->m_socket, (const char*)data.data(), data.size(), 0);

	if (static_cast<size_t>(bytesSent) != data.size())
		throw std::runtime_error("network error while sending data: code " + std::to_string(getAPIError()));
}

std::vector<uint8_t> Connection::read() {
	return this->read(this->ReadChunkSize);
}

std::vector<uint8_t> Connection::read(size_t expectedSize) {

	if (this->m_socket == INVALID_SOCKET)
		throw std::runtime_error("can't read from a closed connection");

	std::lock_guard<std::mutex> lock(this->m_readMutex);

	std::vector<uint8_t> chunk;
	chunk.resize(expectedSize);

	auto bytesReceived = recv(this->m_socket, (char*)chunk.data(), chunk.size(), 0);

	if (bytesReceived == 0) {

		this->end();
		return {};

	} else if (bytesReceived < 0) {

		auto apiError = getAPIError();

		switch (apiError) {

			case LNE_TIMEDOUT: {
				this->end();
				return {};
			} break;
			
			default:
				throw std::runtime_error("network error while getting data: code " + std::to_string(apiError));
		}		
	}

	chunk.resize(bytesReceived);
	chunk.shrink_to_fit();

	return chunk;
}
