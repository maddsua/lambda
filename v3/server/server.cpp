#include "./server.hpp"
#include "./pipelines.hpp"

#include <stdexcept>
#include <thread>

using namespace Lambda;

void Server::serve() {

	if (this->m_active) {
		throw std::runtime_error("Lambda::Serve: Already running");
	}

	if (!this->m_handler) {
		throw std::runtime_error("Lambda::Serve: Handler is not defined");
	}

	this->m_active = true;
	this->m_exit = false;

	//	create tcp listener
	this->m_tcp.options.port = this->options.port;
	this->m_tcp.bind_and_listen();

	//	start connection loop
	this->m_loop = std::async([&]() {

		while (this->m_tcp.is_listening()) {

			try {

				auto next = this->m_tcp.next();
				//	todo: do refernce counting for the context
				std::thread(Pipelines::H1::serve_conn, std::move(next), this->m_handler, ServerContext(this)).detach();

			} catch(const std::exception& e) {

				if (this->m_exit) {
					break;
				}

				fprintf(stderr, "Lambda::Serve: Listnener exception: %s\n", e.what());
			}
		}
	});

	this->m_loop.get();
}

void Server::shutdown() {

	if (!this->m_active) {
		return;
	}

	this->m_exit = false;

	this->m_tcp.shutdown();
	
	if (this->m_loop.valid()) {
		this->m_loop.get();
	}

	this->m_active = false;
}


Server::~Server() {
	this->shutdown();
}
