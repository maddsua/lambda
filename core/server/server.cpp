
#include "./server.hpp"
#include "./http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::HTTPServer;

ServerInstance::ServerInstance(HTTPRequestCallback handlerCallback, ServerConfig init) {

	this->config = init;
	this->handler = handlerCallback;

	Network::TCP::ListenConfig listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.service.fastPortReuse;
	listenInitOpts.port = this->config.service.port;
	this->listener = new Network::TCP::ListenSocket(listenInitOpts);

	this->watchdogWorker = std::thread([&]() {

		while (!this->terminated && this->listener->active()) {

			try {

				auto nextConn = this->listener->acceptConnection();
				if (!nextConn.has_value()) break;
				auto connectionWorker = std::thread(connectionHandler, std::move(nextConn.value()), this->handler, this->config);
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
