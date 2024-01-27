
#include "./server.hpp"
#include "./http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

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
				auto connectionWorker = std::thread(connectionHandler, std::move(nextConn), this->handler, this->config);
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

void ServerInstance::softShutdownn() {

	printf("[Service] Initiating graceful shutdown...\n");

	this->terminated = true;
	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();

	printf("[Service] Server shut down\n");
}

void ServerInstance::immediateShutdownn() {

	printf("[Service] Terminating server now\n");

	this->terminated = true;
	this->listener->stop();

	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();
}

void ServerInstance::awaitFinished() {
	if (this->watchdogWorker.joinable())
		this->watchdogWorker.join();
}

ServerInstance::~ServerInstance() {
	this->immediateShutdownn();
}

const ServerConfig& ServerInstance::getConfig() const noexcept {
	return this->config;
}
