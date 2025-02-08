#include "./net.hpp"
#include "./net_private.hpp"

using namespace Lambda::Net;

//	todo: support ipv6

void TcpListener::bind_and_listen() {

	//	special threatment for windows and it's fucking WSA
	#ifdef _WIN32
		lambda_wsa_wake_up();
	#endif

	this->m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpListener: Unable to create socket", lambda_os_errno());
	}

	if (this->options.fast_port_reuse) {
		const uint32_t opt_reuse = 1;
		if (setsockopt(this->m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&(opt_reuse), sizeof(opt_reuse))) {
			const auto error = Net::Error("TcpListener: Failed to set fast port reuse", lambda_os_errno());
			lambda_close_sock(this->m_sock);
			throw error;
		}
	}

	if (this->options.bind_addr.empty()) {
		throw Net::Error("TcpListener: bind_addr is undefined");
	}

	sockaddr_in listen_addr {
		.sin_family = AF_INET,
		.sin_port = htons(this->options.port),
		.sin_addr = {
			.s_addr = inet_addr(this->options.bind_addr.c_str()),
		},
	};

	if (bind(this->m_sock, (sockaddr*)&listen_addr, sizeof(listen_addr))) {
		const auto error = Net::Error("TcpListener: Failed to bind socket", lambda_os_errno());
		lambda_close_sock(this->m_sock);
		throw error;
	}

	if (listen(this->m_sock, SOMAXCONN)) {
		const auto error = Net::Error("TcpListener: Failed to start listening", lambda_os_errno());
		lambda_close_sock(this->m_sock);
		throw error;
	}
}

TcpListener::TcpListener(TcpListener&& other) noexcept {

	//	transfer socket
	this->m_sock = other.m_sock;
	other.m_sock = LAMBDA_INVALID_SOCKET;

	//	copy options
	this->options = other.options;
}

TcpListener::~TcpListener() {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		return;
	}

	lambda_shutdown_sock(this->m_sock, LAMBDA_SD_BOTH);
	lambda_close_sock(this->m_sock);
}

TcpConnection TcpListener::next() {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpListener: Next on closed socket", lambda_os_errno());
	}

	//	accept network connection
	sockaddr_in next_addr;
	socklen_t next_addr_len = sizeof(next_addr);
	auto next_sock = accept(this->m_sock, (sockaddr*)&next_addr, &next_addr_len);

	//	verify that we have a valid socket
	if (next_sock == LAMBDA_INVALID_SOCKET) {
		throw Net::Error("TcpListener: Accept terminated", lambda_os_errno());
	}

	//	try getting peer host name
	char remote_addr_buff_v4[32];
	auto remote_addr_ptr = inet_ntop(AF_INET, &next_addr.sin_addr, remote_addr_buff_v4, sizeof(remote_addr_buff_v4));

	RemoteAddress remote_addr = {
		.hostname = remote_addr_ptr ? remote_addr_ptr : "",
		.port = ntohs(next_addr.sin_port),
		.transport = Transport::TCP
	};

	return TcpConnection(next_sock, remote_addr);
}

bool TcpListener::is_listening() const noexcept {
	return this->m_sock != LAMBDA_INVALID_SOCKET;
}

void TcpListener::shutdown() {

	if (this->m_sock == LAMBDA_INVALID_SOCKET) {
		return;
	}

	if (lambda_shutdown_sock(this->m_sock, LAMBDA_SD_BOTH)) {
		throw Net::Error("TcpListener: Shutdown error", lambda_os_errno());
	}

	if (lambda_close_sock(this->m_sock)) {
		throw Net::Error("TcpListener: Socket close error", lambda_os_errno());
	}

	this->m_sock = LAMBDA_INVALID_SOCKET;
}
