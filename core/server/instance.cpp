#include "./server.hpp"
#include "./server_impl.hpp"
#include "../network/tcp/listener.hpp"
#include "../http/transport.hpp"

#include <cstdio>
#include <thread>

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

using namespace std::chrono_literals;

LambdaInstance::LambdaInstance(RequestCallback handlerCallback, ServerConfig init) :
	listener({
		init.service.fastPortReuse,
		init.service.port,
		init.service.connectionTimeouts 
	}),
	config(init),
	httpHandler(handlerCallback)
{

	if (init.service.fastPortReuse) {
		syncout.log("[Service] Warning: fast port reuse enabled");
	}

	if (init.service.maxConnections < ServiceOptions::minConnections) {
		throw Lambda::Error("ServiceOptions::maxConnections value is lower than allowed by minConnections");
	}

	this->serviceWorker = std::async([&]() {

		if (this->config.service.useThreadList) {

			const auto workerJoinFilter = [&](WorkerContext& node) -> bool {
				if (!node.finished) {
					return false;
				}

				if (node.worker.joinable()) {
					node.worker.join();
				}

				this->m_connections_count--;
				return true;
			};

			const auto& svcmaxconn = this->config.service.maxConnections;
			const auto gcThreshold = svcmaxconn * 0.75;

			while (!this->m_terminated && this->listener.active()) {

				auto nextConn = this->listener.acceptConnection();
				if (!nextConn.has_value()) break;

				if (svcmaxconn && (this->m_connections_count > svcmaxconn)) {
					nextConn.value().end();
					this->m_connections.remove_if(workerJoinFilter);
					continue;
				}

				this->m_connections.push_front({
					std::move(nextConn.value())
				});
				this->m_connections_count++;

				auto& nextWorker = this->m_connections.front();
				nextWorker.worker = std::thread([&](WorkerContext& worker) {
					connectionWorker(worker, this->config, this->httpHandler);
					worker.finished = true;
				}, std::ref(nextWorker));

				if (!svcmaxconn || this->m_connections_count > gcThreshold) {
					this->m_connections.remove_if(workerJoinFilter);
				}
			}

		} else {

			while (!this->m_terminated && this->listener.active()) {

				auto nextConn = this->listener.acceptConnection();
				if (!nextConn.has_value()) break;

				std::thread([&](WorkerContext&& worker) {
					connectionWorker(worker, this->config, this->httpHandler);
				}, WorkerContext {
					std::move(nextConn.value())
				}).detach();
			}
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

	//	reqeust service worker to exit
	this->m_terminated = true;
	
	//	close listen socket
	this->listener.stop();

	//	Request all connection workers to exit
	if (this->config.service.useThreadList) {
		for (auto& worker : this->m_connections) {
			worker.shutdownFlag = true;
		}
	}
}

void LambdaInstance::awaitFinished() {

	//	wait until service worker exits
	if (this->serviceWorker.valid())
		this->serviceWorker.get();

	//	Wait until all connection workers done
	if (this->config.service.useThreadList) {
		for (auto& item : this->m_connections) {
			if (item.worker.joinable()) {
				item.worker.join();
			}
		}
	}
}

LambdaInstance::~LambdaInstance() {

	//	send terminate "signals"
	this->terminate();

	//	Wait until service worker exits.
	//	Copypasted a bit but it's better than adding arguments to awaitFinished()
	//	just to accound for exception suppression
	if (this->serviceWorker.valid()) {
		try { this->serviceWorker.get(); }
			catch(...) {}
	}

	//	Wait until all connection workers exited
	if (this->config.service.useThreadList) {
		for (auto& item : this->m_connections) {
			if (item.worker.joinable()) {
				item.worker.join();
			}
		}
	}
}

const ServerConfig& LambdaInstance::getConfig() const noexcept {
	return this->config;
}
