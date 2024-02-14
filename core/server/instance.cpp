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

			if (!svcmaxconn || this->m_connections_count > svcmaxconn) {
				this->m_connections.remove_if(workerJoinFilter);
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
	for (auto& worker : this->m_connections) {
		worker.shutdownFlag = true;
	}
}

void LambdaInstance::awaitFinished() {

	//	wait until service worker exits
	if (this->serviceWorker.valid())
		this->serviceWorker.get();

	//	Wait until all connection workers done
	for (auto& item : this->m_connections) {
		if (item.worker.joinable()) {
			item.worker.join();
		}
	}
}

LambdaInstance::~LambdaInstance() {

	//	send terminate "signals"
	this->terminate();

	//	wait till service worker exits and suppress all errors
	if (this->serviceWorker.valid()) {
		try { this->awaitFinished(); }
			catch(...) {}
	}
}

const ServerConfig& LambdaInstance::getConfig() const noexcept {
	return this->config;
}
