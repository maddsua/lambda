#include <iostream>
#include <thread>

#include "../server.hpp"

using namespace Lambda;

HttpServer::HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerInit init) {

	this->config = init;
	this->handler = handlerFunction;

	Network::ListenInit listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.fastPortReuse;
	auto tempListener = Network::TCPListenSocket(this->config.port, listenInitOpts);
	this->listener = new Network::TCPListenSocket(std::move(tempListener));

	this->watchdogRoutine = std::async([&]() {

		while (!this->terminated && this->listener->ok()) {

			try {

				auto conn = this->listener->acceptConnection();
			
				std::cout << "Got a connection!\n-----\n";

				//auto worker = std::thread(Server::handleHTTPConnection, std::move(conn), this->handler);

				auto temp = std::async(Server::handleHTTPConnection, std::move(conn), this->handler);

				//Server::handleHTTPConnection(conn, this->handler);
				std::cout << "TCP connection served and closed\n-----\n";

			} catch(const std::exception& e) {
				if (!this->terminated) return;
				std::cerr << "http handler crashed: " << e.what() << '\n';
			} catch(...) {
				if (!this->terminated) return;
				std::cerr << "http handler crashed: unhandled error\n";
			}
		}
	});
};

HttpServer::~HttpServer() {
	this->terminated = false;
	delete this->listener;
	this->watchdogRoutine.get();
}

const HttpServerInit& HttpServer::getConfig() const noexcept {
	return this->config;
}
