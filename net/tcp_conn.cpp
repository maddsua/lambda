#include "./net.hpp"
#include "./net_private.hpp"

#include <sys/types.h>
#include <sys/socket.h>

using namespace Lambda::Net;

uint32_t gen_conn_id(SockHandle sock, uint16_t port) {
	uint64_t big_num = sock * port;
	return (big_num & 0xffffffff) ^ ((big_num >> 32) & 0xffffffff);
}

TcpConnection::TcpConnection(SockHandle sock_handle, const RemoteAddress& remote) {

	if (sock_handle == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpConnection: Invalid socket handle");
	}

	this->m_sock = sock_handle;
	this->m_remote_addr = remote;
	this->m_id = gen_conn_id(this->m_sock, this->m_remote_addr.port);
}

TcpConnection::TcpConnection(TcpConnection&& other) noexcept {
	
	//	transfer socket ownership
	this->m_sock = other.m_sock;
	other.m_sock = LAMBDA_INVALID_SOCKET;

	//	copy metadata
	this->m_remote_addr = other.m_remote_addr;
	this->m_timeouts = other.m_timeouts;
	this->m_id = other.m_id;
}

TcpConnection::~TcpConnection() {
	
	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		return;
	}

	lambda_shutdown_sock(this->m_sock, LAMBDA_SD_BOTH);
	lambda_close_sock(this->m_sock);
}

const RemoteAddress& TcpConnection::remote_addr() const noexcept {
	return this->m_remote_addr;
}

const ConnectionTimeouts& TcpConnection::timeouts() const noexcept {
	return this->m_timeouts;
}

std::vector<uint8_t> TcpConnection::read(size_t chunk_size) {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpConnection: Read on a closed socket");
	}

	auto dest = std::vector<uint8_t>(chunk_size);
	auto bytes_read = recv(this->m_sock, reinterpret_cast<char*>(dest.data()), dest.size(), 0);

	if (bytes_read > 0) {
		dest.resize(bytes_read);
		return dest;
	} else if (bytes_read == 0) {
		this->close();
		return {};
	}

	auto err_code = lambda_os_errno();

	if (lambda_is_error_timeout(err_code)) {
		return {};
	}

	if (lambda_is_error_disconnect(err_code)) {
		this->close();
		return {};
	}

	throw Net::Error("TcpConnection: Error reading data", err_code);
}

size_t TcpConnection::write(const std::vector<uint8_t>& data) {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpConnection: Write on a closed socket");
	}

	auto bytes_sent = send(this->m_sock, (const char*)data.data(), data.size(), MSG_NOSIGNAL);
	if (static_cast<size_t>(bytes_sent) == data.size()) {
		return bytes_sent;
	}

	auto err_code = lambda_os_errno();

	if (lambda_is_error_disconnect(err_code)) {
		this->close();
		return {};
	}

	throw Net::Error("TcpConnection: Error writing data", err_code);
}

void TcpConnection::set_timeouts(ConnectionTimeouts timeouts) {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpConnection: Set timeouts on closed socket");
	}

	//	ensure timeouts arent negative
	if (timeouts.read <= 0) {
		throw Net::Error("TcpConnection: Set timeouts: RX value invalid");
	}
	if (timeouts.write <= 0) {
		throw Net::Error("TcpConnection: Set timeouts: TX value invalid");
	}

	auto cast_timeout_value = [](uint32_t value_ms) {
		#ifndef _WIN32
			timeval value = {
				.tv_sec = value_ms / 1000,
				.tv_usec = 0
			};
			return value;
		#else
			return value_ms;
		#endif
	};

	//	only update modified values
	if (timeouts.read != this->m_timeouts.read) {
		auto value = cast_timeout_value(timeouts.read);
		if (setsockopt(this->m_sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&value), sizeof(value))) {
			throw Net::Error("TcpConnection: Error setting read timeout", lambda_os_errno());
		}
		this->m_timeouts.read = timeouts.read;
	}

	if (timeouts.write != this->m_timeouts.write) {
		auto value = cast_timeout_value(timeouts.write);
		if (setsockopt(this->m_sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&value), sizeof(value))) {
			throw Net::Error("TcpConnection: Error setting write timeout", lambda_os_errno());
		}
		this->m_timeouts.write = timeouts.write;
	}
}

void TcpConnection::close() noexcept {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		return;
	}

	lambda_shutdown_sock(this->m_sock, LAMBDA_SD_BOTH);
	lambda_close_sock(this->m_sock);

	this->m_sock = LAMBDA_INVALID_SOCKET;
}

bool TcpConnection::is_open() const noexcept {
	return this->m_sock != LAMBDA_INVALID_SOCKET;
}

uint32_t TcpConnection::id() const noexcept {
	return this->m_id;
}
