#include "./server.hpp"
#include "./pipelines.hpp"
#include "../log/log.hpp"

#include <stdexcept>
#include <thread>

using namespace Lambda;

std::string parse_bind_addr(const std::string& input);

Server::Server(HandlerFn handler_fn) : m_handler_fn(handler_fn) {
	if (!this->m_handler_fn) {
		throw std::runtime_error("Lambda::Serve: Root handler is not defined");
	}
}

Server::Server(HandlerFn handler_fn, ServeOptions options) : m_handler_fn(handler_fn), options(options) {
	if (!this->m_handler_fn) {
		throw std::runtime_error("Lambda::Serve: Root handler is not defined");
	}
}

Server::Server(std::shared_ptr<Handler> handler) : m_handler(handler) {

	if (!this->m_handler) {
		throw std::runtime_error("Lambda::Serve: Root handler is not defined");
	}

	this->m_handler_fn = std::bind(&Handler::handler_fn, this->m_handler.get(), std::placeholders::_1, std::placeholders::_2);
}

Server::Server(std::shared_ptr<Handler> handler, ServeOptions options) : m_handler(handler), options(options) {

	if (!this->m_handler) {
		throw std::runtime_error("Lambda::Serve: Root handler is not defined");
	}

	this->m_handler_fn = std::bind(&Handler::handler_fn, this->m_handler.get(), std::placeholders::_1, std::placeholders::_2);
}

void Server::serve() {

	if (!*this->m_done) {
		throw std::runtime_error("Lambda::Serve: Already running");
	}

	*this->m_done = false;

	//	create tcp listener
	this->m_tcp.options.port = this->options.port;
	this->m_tcp.options.bind_addr = parse_bind_addr(this->options.host_addr);
	this->m_tcp.bind_and_listen();

	//	start connection loop
	this->m_loop = std::async([&]() {

		while (this->m_tcp.is_listening()) {

			try {

				auto next = this->m_tcp.next();
				auto ctx = Pipelines::ServeContext(this->options, this->m_done);

				std::thread( Pipelines::H1::serve_conn, std::move(next), this->m_handler_fn, ctx).detach();

			} catch(const std::exception& e) {

				if (this->m_done) {
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

	if (*this->m_done) {
		return;
	}

	*this->m_done = true;

	this->m_tcp.shutdown();

	if (this->m_loop.valid()) {
		this->m_loop.get();
	}
}

Server::~Server() {
	this->shutdown();
	*this->m_done = true;
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
