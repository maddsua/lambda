#include "./connection.hpp"
#include "../sysnetw.hpp"

using namespace Lambda::Network;
using namespace Lambda::Network::TCP;

Connection::Connection(ConnCreateInit init) {
	this->info = init.info;
	this->hSocket = init.hSocket;
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

void Connection::end() {
	if (this->hSocket == INVALID_SOCKET) return;
	shutdown(this->hSocket, SD_BOTH);
	closesocket(this->hSocket);
	this->hSocket = INVALID_SOCKET;
}

const ConnectionInfo& Connection::getInfo() const noexcept {
	return this->info;
}

bool Connection::isOpen() const noexcept {
	return this->hSocket != INVALID_SOCKET;
}

void Connection::write(const std::vector<uint8_t>& data) {

	if (this->hSocket == INVALID_SOCKET)
		throw std::runtime_error("cann't write to a closed connection");

	std::lock_guard<std::mutex> lock(this->writeMutex);

	auto bytesSent = send(this->hSocket, (const char*)data.data(), data.size(), 0);

	if (static_cast<size_t>(bytesSent) != data.size())
		throw std::runtime_error("network error while sending data: code " + std::to_string(getAPIError()));
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
