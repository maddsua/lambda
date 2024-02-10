#include "./server.hpp"
#include "./internal.hpp"
#include "../network/tcp/listener.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::Server::Handlers;

LambdaInstance::LambdaInstance(
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

LambdaInstance::LambdaInstance(
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

void LambdaInstance::start() {

	if (this->config.service.fastPortReuse) {
		syncout.log("Warning: fast port reuse enabled");
	}

	this->watchdogWorker = std::async([&]() {

		while (!this->m_terminated && this->listener.active()) {

			auto nextConn = this->listener.acceptConnection();
			if (!nextConn.has_value()) break;

			std::thread([&](Lambda::Network::TCP::Connection&& conn) {

				const auto& conninfo = conn.info();
				std::optional<std::string> handlerError;

				if (this->config.loglevel.connections) {
					syncout.log({
						conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
						"connected on",
						conninfo.hostPort
					});
				}

				try {

					switch (this->handlerType) {

						case HandlerType::Serverless: {
							serverlessHandler(conn, this->config, this->httpHandler);
						} break;

						case HandlerType::Connection: {
							streamHandler(conn, this->config, this->tcpHandler);
						} break;

						default: {
							this->m_terminated = true;
							throw std::runtime_error("connection handler undefined");
						} break;
					}

				} catch(const std::exception& e) {
					handlerError = e.what();
				} catch(...) {
					handlerError = "unknown error";
				}

				if (handlerError.has_value() && (config.loglevel.connections || config.loglevel.requests)) {

					syncout.error({
						"[Service] Connection to",
						conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
						"terminated",
						'(' + handlerError.value() + ')'
					});

				} else if (config.loglevel.connections) {

					syncout.log({
						conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
						"disconnected from",
						conninfo.hostPort
					});
				}

			}, std::move(nextConn.value())).detach();
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
