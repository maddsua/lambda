#include "./server.hpp"
#include "./pipelines.hpp"
#include "../log/log.hpp"

#include <stdexcept>
#include <thread>

using namespace Lambda;

std::string parse_bind_addr(const std::string& input);

void Server::serve() {

	if (this->m_active) {
		throw std::runtime_error("Lambda::Serve: Already running");
	}

	if (!this->m_handler) {
		throw std::runtime_error("Lambda::Serve: Handler is not defined");
	}

	this->m_active = true;
	*this->m_exit = false;

	//	create tcp listener
	this->m_tcp.options.port = this->options.port;
	this->m_tcp.options.bind_addr = parse_bind_addr(this->options.host_addr);
	this->m_tcp.bind_and_listen();

	//	start connection loop
	this->m_loop = std::async([&]() {

		while (this->m_tcp.is_listening()) {

			try {

				auto next = this->m_tcp.next();
				auto ctx = ServeContext(this->options, this->m_exit);

				std::thread(Pipelines::H1::serve_conn, std::move(next), this->m_handler, ctx).detach();

			} catch(const std::exception& e) {

				if (this->m_exit) {
					break;
				}

				Log::err("{} ERROR Lambda::Serve Listnener exception: {}", {
					Date().to_log_string(),
					e.what()
				});
			}
		}
	});

	Log::err("{} DEBUG Lambda::Serve Listening at http://{}:{}/", {
		Date().to_log_string(),
		this->options.host_addr.empty() ? "localhost" : this->options.host_addr,
		this->options.port
	});

	this->m_loop.get();
}

void Server::shutdown() {

	if (!this->m_active) {
		return;
	}

	*this->m_exit = true;

	this->m_tcp.shutdown();
	
	if (this->m_loop.valid()) {
		this->m_loop.get();
	}

	this->m_active = false;
}


Server::~Server() {
	this->shutdown();
	*this->m_exit = true;
}

std::string parse_bind_addr(const std::string& addr) {

	if (addr.empty()) {
		return "0.0.0.0";
	}

	for (auto rune : addr) {
		if (!((rune >= '0' && rune <= '9') || rune == '.')) {
			throw std::runtime_error("Bind address '" + addr + "' is not valid");
		}
	}

	auto assert_segment = [&](const std::string& token) {

		int64_t value = -1;

		try { value = std::stoll(token); }
			catch(...) { }

		if (value < 0 || value > 255) {
			throw std::runtime_error("Bind address '" + addr + "' is not valid");
		}
	};

	size_t segment_begin = 0;
	size_t segments_count = 0;
	for (size_t idx = 0; idx < addr.size(); idx++) {
		if (addr[idx] == '.') {
			assert_segment(addr.substr(segment_begin, idx - segment_begin));
			segment_begin = idx + 1;
			segments_count++;
		}
	}

	assert_segment(addr.substr(segment_begin));

	if (segments_count != 3) {
		throw std::runtime_error("Bind address '" + addr + "' is not valid");
	}

	return addr;
}
