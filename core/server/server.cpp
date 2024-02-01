
#include "./server.hpp"
#include "./http.hpp"
#include "./handlers.hpp"
#include "../crypto/crypto.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::HTTPServer;
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

	this->watchdogWorker = std::async([&]() {

		while (!this->terminated && this->listener->active()) {

			auto nextConn = this->listener->acceptConnection();
			if (!nextConn.has_value()) break;

			if (this->httpHandler) {
				auto connectionWorker = std::thread(serverlessHandler,
					std::move(nextConn.value()),
					std::ref(this->config),
					std::ref(this->httpHandler));
				connectionWorker.detach();
			} else {
				auto connectionWorker = std::thread(connectionHandler,
					std::move(nextConn.value()),
					std::ref(this->config),
					std::ref(this->tcpHandler));
				connectionWorker.detach();
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
	if (this->watchdogWorker.valid())
		this->watchdogWorker.get();
}

ServerInstance::~ServerInstance() {
	this->terminate();
	delete this->listener;
}

const ServerConfig& ServerInstance::getConfig() const noexcept {
	return this->config;
}
