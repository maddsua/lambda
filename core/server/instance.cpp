#include "./server.hpp"
#include "./http/http.hpp"
#include "./handlers/handlers.hpp"
#include "../crypto/crypto.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::HTTPServer;
using namespace Lambda::Server::Handlers;

ServerInstance::ServerInstance(
	ServerlessCallback handlerCallback,
	ServerConfig init
) : listener({
	init.service.fastPortReuse,
	init.service.port,
	init.service.connectionTimeout
}) {
	this->config = init;
	this->httpHandler = handlerCallback;
	this->handlerType = HandlerType::Serverless;
	this->start();
}

ServerInstance::ServerInstance(
	ConnectionCallback handlerCallback,
	ServerConfig init
) : listener({
	init.service.fastPortReuse,
	init.service.port,
	init.service.connectionTimeout
}) {
	this->config = init;
	this->tcpHandler = handlerCallback;
	this->handlerType = HandlerType::Connection;
	this->start();
}

void ServerInstance::start() {
	this->watchdogWorker = std::async([&]() {

		while (!this->terminated && this->listener.active()) {

			auto nextConn = this->listener.acceptConnection();
			if (!nextConn.has_value()) break;

			switch (this->handlerType) {

				case HandlerType::Serverless: {
					auto connectionWorker = std::thread(serverlessHandler,
						std::move(nextConn.value()),
						std::ref(this->config),
						std::ref(this->httpHandler));
					connectionWorker.detach();
				} break;

				case HandlerType::Connection: {
					auto connectionWorker = std::thread(connectionHandler,
						std::move(nextConn.value()),
						std::ref(this->config),
						std::ref(this->tcpHandler));
					connectionWorker.detach();
				} break;

				default:
					throw std::runtime_error("ServerInstance cannot invode an undefined handlerCallback");
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
	this->listener.stop();
	this->awaitFinished();
}

void ServerInstance::awaitFinished() {
	if (this->watchdogWorker.valid())
		this->watchdogWorker.get();
}

ServerInstance::~ServerInstance() {
	this->terminate();
}

const ServerConfig& ServerInstance::getConfig() const noexcept {
	return this->config;
}
