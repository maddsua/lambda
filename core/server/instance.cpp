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
		syncout.log("[Service] Warning: fast port reuse enabled");
	}

	this->watchdogWorker = std::async([&]() {

		while (!this->m_terminated && this->listener.active()) {

			auto nextConn = this->listener.acceptConnection();
			if (!nextConn.has_value()) break;

			std::thread([&](Lambda::Network::TCP::Connection&& conn) {

				const auto& conninfo = conn.info();
				auto connctx = IncomingConnection(conn, this->config);

				if (this->config.loglevel.transportEvents) {
					syncout.log({
						"[Transport]",
						conninfo.remoteAddr.hostname + ':' + std::to_string(conninfo.remoteAddr.port),
						"created",
						connctx.contextID().toString()
					});
				}

				const auto displayHandlerCrashMessage = [&](const std::string& message) {

					if (!(config.loglevel.transportEvents || config.loglevel.requests)) return;

					auto transportDisplayID = connctx.contextID().toString();
					if (!this->config.loglevel.transportEvents) {
						transportDisplayID += '(' + conninfo.remoteAddr.hostname + 
							':' + std::to_string(conninfo.remoteAddr.port) + ')';
					}

					syncout.error({
						"[Transport]",
						transportDisplayID,
						"terminated:",
						message
					});
				};

				try {

					switch (this->handlerType) {

						case HandlerType::Serverless: {
							serverlessHandler(connctx, this->config, this->httpHandler);
						} break;

						case HandlerType::Connection: {
							streamHandler(connctx, this->config, this->tcpHandler);
						} break;

						default: {
							this->m_terminated = true;
							throw Lambda::Error("Instance handler undefined");
						} break;
					}

				} catch(const std::exception& err) {
					displayHandlerCrashMessage(err.what());
					return;
				} catch(...) {
					displayHandlerCrashMessage("Unknown exception");
					return;
				}

				if (config.loglevel.transportEvents) {
					syncout.log({
						"[Transport]",
						connctx.contextID().toString(),
						"closed ok"
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
