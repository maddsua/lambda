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
	listener({ init.service.fastPortReuse, init.service.port, init.service.connectionTimeouts }),
	config(init), httpHandler(handlerCallback) {

	if (this->config.service.fastPortReuse) {
		syncout.log("[Service] Warning: fast port reuse enabled");
	}

	this->serviceWorker = std::async([&]() {

		const auto workersGCJob = [&]() -> void {
			this->m_connections.remove_if([&](WorkerContext& node) {

				if (!node.finished) {
					return false;
				}

				if (node.worker.joinable()) {
					node.worker.join();
				}

				this->m_connections_count--;
				return true;
			});
		};

		const auto& svcmaxconn = this->config.service.maxConnections;
		while (!this->m_terminated && this->listener.active()) {

			auto nextConn = this->listener.acceptConnection();
			if (!nextConn.has_value()) break;

			if (svcmaxconn && (this->m_connections_count > svcmaxconn)) {
				nextConn.value().end();
				workersGCJob();
				std::this_thread::sleep_for(1ms);
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

			workersGCJob();
		}

		if (this->m_terminated) {
			for (auto& worker : this->m_connections) {
				worker.shutdownFlag = true;
			}
		}

		for (auto& item : this->m_connections) {
			if (item.worker.joinable()) {
				item.worker.join();
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
	this->m_terminated = true;
	this->listener.stop();
	this->awaitFinished();
}

void LambdaInstance::awaitFinished() {
	if (this->serviceWorker.valid())
		this->serviceWorker.get();
}

LambdaInstance::~LambdaInstance() {
	this->terminate();
}

const ServerConfig& LambdaInstance::getConfig() const noexcept {
	return this->config;
}
