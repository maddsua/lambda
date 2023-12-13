#include <cstdio>
#include <thread>

#include "../server.hpp"
#include "../polyfill.hpp"
#include "../crypto.hpp"

using namespace Lambda;
using namespace Lambda::Server;

HttpServer::HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerInit init) {

	this->config = init;
	this->handler = handlerFunction;

	Network::ListenInit listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.fastPortReuse;
	auto tempListener = Network::TCPListenSocket(this->config.port, listenInitOpts);
	this->listener = new Network::TCPListenSocket(std::move(tempListener));

	this->watchdogWorker = std::thread([&]() {

		while (!this->terminated && this->listener->ok()) {

			try {

				auto nextConn = this->listener->acceptConnection();

				auto connectionWorker = std::thread([&](Network::TCPConnection&& conn) {

					try {

						Server::handleHTTPConnection(std::move(conn), this->handler, this->config.handlerOptions);

					} catch(const std::exception& e) {
						if (this->terminated) return;
						fprintf(stderr, "[Service] http handler crashed: %s\n", e.what()); 
					} catch(...) {
						if (this->terminated) return;
						fprintf(stderr, "[Service] http handler crashed with unknown error\n"); 
					}

				}, std::move(nextConn));

				connectionWorker.detach();

			} catch(const std::exception& e) {
				if (this->terminated) return;
				fprintf(stderr, "[Service] connection handler crashed: %s\n", e.what()); 
			} catch(...) {
				if (this->terminated) return;
				fprintf(stderr, "[Service] connection handler crashed with unknown error\n"); 
			}
		}
	});

	printf("[Service] Started server at http://localhost:%i/\n", this->config.port);
};

void HttpServer::softShutdownn() {

	printf("[Service] Initiating graceful shutdown...\n");

	this->terminated = true;
	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();

	printf("[Service] Server shut down\n");
}

void HttpServer::immediateShutdownn() {

	printf("[Service] Terminating server now\n");

	this->terminated = true;
	delete this->listener;

	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();
}

void HttpServer::awaitFinished() {
	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();
}

HttpServer::~HttpServer() {
	this->immediateShutdownn();
}

const HttpServerInit& HttpServer::getConfig() const noexcept {
	return this->config;
}
