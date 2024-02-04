#include "./server.hpp"
#include "./internal.hpp"
#include "../crypto/crypto.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::Server::Handlers;

ServerInstance::ServerInstance(
	ServerlessCallback handlerCallback,
	ServerConfig init
) : listener({
	init.service.fastPortReuse,
	init.service.port,
	init.service.connectionTimeouts
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
	init.service.connectionTimeouts
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

			std::thread([&](Lambda::Network::TCP::Connection&& conn) {

				const auto& conninfo = conn.info();
				std::optional<std::string> handlerError;

				if (this->config.loglevel.connections) {
					syncout.log({ conninfo.remoteAddr.hostname, ":", conninfo.remoteAddr.port, "connected on", conninfo.hostPort });
				}

				try {

					switch (this->handlerType) {

						case HandlerType::Serverless: {
							serverlessHandler(conn, this->config, this->httpHandler);
						} break;

						case HandlerType::Connection: {
							connectionHandler(conn, this->config, this->tcpHandler);
						} break;

						default: {
							this->terminated = true;
							throw std::runtime_error("connection handler undefined");
						} break;
					}

				} catch(const std::exception& e) {
					handlerError = e.what();
				} catch(...) {
					handlerError = "unknown error";
				}

				if (handlerError.has_value() && (config.loglevel.connections || config.loglevel.requests)) {
					syncout.log({ "[Service] Connection to", conninfo.remoteAddr.hostname, "terminated (", handlerError.value(), ')' });
				} else if (config.loglevel.connections) {
					syncout.log({ conninfo.remoteAddr.hostname, ":", conninfo.remoteAddr.port, "disconnected from", conninfo.hostPort });
				}

			}, std::move(nextConn.value())).detach();
		}
	});

	syncout.log({ "[Service] Started server at http://localhost:", this->config.service.port, '/' });
}

void ServerInstance::shutdownn() {
	syncout.log("[Service] Shutting down...");
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
