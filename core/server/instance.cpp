#include "./server.hpp"
#include "./server_impl.hpp"
#include "../network/tcp/listener.hpp"
#include "../http/transport.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

LambdaInstance::LambdaInstance(RequestCallback handlerCallback, ServerConfig init) :
	listener({ init.service.fastPortReuse, init.service.port, init.service.connectionTimeouts }),
	config(init), httpHandler(handlerCallback) {

	if (this->config.service.fastPortReuse) {
		syncout.log("[Service] Warning: fast port reuse enabled");
	}

	this->watchdogWorker = std::async([&]() {

		while (!this->m_terminated && this->listener.active()) {

			auto nextConn = this->listener.acceptConnection();
			if (!nextConn.has_value()) break;

			std::thread(Server::connectionHandler,
				std::move(nextConn.value()),
				std::ref(this->config),
				std::ref(this->httpHandler)
			).detach();
		}
	});

	if (config.loglevel.startMessage) {
		syncout.log("[Service] Started at http://localhost:" + std::to_string(this->config.service.port) + '/');
	}
}

void LambdaInstance::shutdownn() {
	syncout.log("[Service] Shutting down...");
	this->terminate();
}

void LambdaInstance::terminate() {
	this->m_terminated = true;
	this->listener.stop();
	this->awaitFinished();
}

void LambdaInstance::awaitFinished() {
	if (this->watchdogWorker.valid())
		this->watchdogWorker.get();
}

LambdaInstance::~LambdaInstance() {
	this->terminate();
}

const ServerConfig& LambdaInstance::getConfig() const noexcept {
	return this->config;
}
