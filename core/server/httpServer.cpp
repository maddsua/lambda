#include <cstdio>
#include <thread>

#include "../server.hpp"
#include "../polyfill.hpp"
#include "../crypto.hpp"
#include "../encoding.hpp"

using namespace Lambda;
using namespace Lambda::Server;

HttpServer::HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerConfig init) {

	this->config = init;
	this->handler = handlerFunction;

	Network::ListenInit listenInitOpts;
	listenInitOpts.allowPortReuse = this->config.service.fastPortReuse;
	auto tempListener = Network::TCPListenSocket(this->config.service.port, listenInitOpts);
	this->listener = new Network::TCPListenSocket(std::move(tempListener));

	this->watchdogWorker = std::thread([&]() {

		while (!this->terminated && this->listener->ok()) {

			try {

				auto nextConn = this->listener->acceptConnection();

				auto connectionWorker = std::thread([&](Network::TCPConnection&& conn) {

					const auto& connfinfo = conn.getInfo();
					auto contextID = connfinfo.shortid.toString();

					HttpHandlerOptions handlerOptions = {
						this->config.loglevel,
						this->config.transport,
						contextID
					};

					try {

						if (this->config.loglevel.logConnections) {
							printf("%s %s opens %s\n",
								Date().toHRTString().c_str(),
								connfinfo.peerIP.c_str(),
								contextID
							);
						}

						Server::handleHTTPConnection(std::move(conn), this->handler, handlerOptions);

					} catch(const std::exception& e) {

						if (this->terminated) return;

						if (this->config.loglevel.logRequests) {
							fprintf(stderr, "%s [Service] http handler %s crashed: %s\n", Date().toHRTString().c_str(), contextID.c_str(), e.what()); 
						}

					} catch(...) {

						if (this->terminated) return;

						if (this->config.loglevel.logRequests) {
							fprintf(stderr, "%s [Service] http handler %s crashed with unknown error\n", Date().toHRTString().c_str(), contextID.c_str()); 
						}
					}

					if (this->config.loglevel.logConnections) {
						printf("%s %s was closed\n", Date().toHRTString().c_str(), contextID.c_str());
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

	printf("[Service] Started server at http://localhost:%i/\n", this->config.service.port);
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

const HttpServerConfig& HttpServer::getConfig() const noexcept {
	return this->config;
}
