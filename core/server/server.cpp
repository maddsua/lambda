
#include "./server.hpp"
#include "./handlers.hpp"
#include "../crypto/crypto.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::Server::Handlers;

ServerInstance::ServerInstance(HTTPRequestCallback handlerCallback, ServerConfig init) {
	this->config = init;
	this->httpHandler = handlerCallback;
	this->setup();
}

ServerInstance::ServerInstance(ConnectionCallback handlerCallback, ServerConfig init) {
	this->config = init;
	this->tcpHandler = handlerCallback;
	this->setup();
}

void ServerInstance::setup() {

	Network::TCP::ListenConfig listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.service.fastPortReuse;
	listenInitOpts.port = this->config.service.port;
	this->listener = new Network::TCP::ListenSocket(listenInitOpts);

	this->watchdogWorker = std::thread([&]() {

		while (!this->terminated && this->listener->active()) {

			try {

				auto nextConn = this->listener->acceptConnection();
				if (!nextConn.has_value()) break;

				auto connectionWorker = std::thread(this->httpHandler,
					std::move(nextConn.value()),
					std::ref(this->config),
					std::ref(this->httpHandler));

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

	printf("[Service] Started server at http://localhost:%i/\n", this->config.service.port);

}

void ServerInstance::shutdownn() {
	printf("[Service] Shutting down...\n");
	this->terminate();
}

void ServerInstance::terminate() {
	this->terminated = true;
	this->listener->stop();
	this->awaitFinished();
}

void ServerInstance::awaitFinished() {
	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();
}

ServerInstance::~ServerInstance() {
	this->terminate();
	delete this->listener;
}

const ServerConfig& ServerInstance::getConfig() const noexcept {
	return this->config;
}
