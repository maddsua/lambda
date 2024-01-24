
#include "./server.hpp"
#include "./http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../crypto/crypto.hpp"
#include "../encoding/encoding.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;

void connectionHandler(Network::TCP::Connection&& conn, Server::Handlers::HandlerFunction handlerCallback, const ServerConfig& config) {

	const auto& connInfo = conn.getInfo();

	if (config.loglevel.connections) fprintf(stdout,
		"%s %s:%i connected on %i\n",
		Date().toHRTString().c_str(),
		connInfo.remoteAddr.hostname.c_str(),
		connInfo.remoteAddr.port,
		connInfo.hostPort
	);

	try {

		//	I want to add an await here soo badly lol
		Server::httpPipeline(std::move(conn), handlerCallback, {
			config.loglevel,
			config.transport
		});

	} catch(const std::exception& e) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s [Service] Connection to %s terminated: %s\n",
			Date().toHRTString().c_str(),
			connInfo.remoteAddr.hostname.c_str(),
			e.what()
		);

	} catch(...) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s [Service] Connection to %s terminated (unknown error)\n",
			Date().toHRTString().c_str(),
			connInfo.remoteAddr.hostname.c_str()
		);
	}

	if (config.loglevel.connections) fprintf(stdout,
		"%s %s:%i disconnected from %i\n",
		Date().toHRTString().c_str(),
		connInfo.remoteAddr.hostname.c_str(),
		connInfo.remoteAddr.port,
		connInfo.hostPort
	);
}

ServerInstance::ServerInstance(Server::Handlers::HandlerFunction handlerCallback, ServerConfig init) {

	this->config = init;
	this->handler = handlerCallback;

	Network::TCP::ListenConfig listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.service.fastPortReuse;
	listenInitOpts.port = this->config.service.port;
	auto tempListener = Network::TCP::ListenSocket(listenInitOpts);
	this->listener = new Network::TCP::ListenSocket(std::move(tempListener));

	this->watchdogWorker = std::thread([&]() {

		while (!this->terminated && this->listener->ok()) {

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
	delete this->listener;

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
