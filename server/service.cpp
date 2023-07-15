#include "./server.hpp"
#include "../http/http.hpp"
#include <chrono>

using namespace Lambda;
using namespace Lambda::Network;

void Server::init() {

	try {
		this->ListenSocketObj = new Network::ListenSocket(this->openOnPort);
	} catch(const std::exception& e) {
		addLogRecord(std::string("Failed to start server: Listen socket error: ") + e.what(), LAMBDA_LOG_ERROR);
		return;
	}

	running = true;
	handlerDispatched = true;
	watchdogThread = new std::thread(connectionWatchdog, this);
	addLogRecord("Server start successful", LAMBDA_LOG_INFO);
}

Server::~Server() {
	running = false;
	if (this->watchdogThread != nullptr) {
		if (watchdogThread->joinable())
			watchdogThread->join();
		delete watchdogThread;
	}
	delete ListenSocketObj;
}

void Server::setServerCallback(void (*callback)(Network::HTTPServer&, Context&)) {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallback = callback;
}
void Server::removeServerCallback() {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallback = nullptr;
}
void Server::setServerlessCallback(HTTP::Response (*callback)(HTTP::Request&, Context&)){
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallbackServerless = callback;
}
void Server::removeServerlessCallback() {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallbackServerless = nullptr;
}

void Server::enablePasstrough(void* object) {
	this->instancePasstrough = object;
}
void Server::disablePasstrough() {
	this->instancePasstrough = nullptr;
}

void Server::connectionWatchdog() {

	auto lastDispatched = std::chrono::steady_clock::now();

	while (running) {

		if (!handlerDispatched) {

			if ((std::chrono::steady_clock::now() - lastDispatched) > std::chrono::milliseconds(1)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			continue;
		}

		if (!ListenSocketObj->isAlive()) {
			
			addLogRecord("Restarting the listen socket...", LAMBDA_LOG_WARN);

			delete this->ListenSocketObj;

			try {
				this->ListenSocketObj = new Network::ListenSocket(this->openOnPort);
			} catch(const std::exception& e) {
				addLogRecord(std::string("Failed to restart the listen socket: ") + e.what() + "Server shut down.", LAMBDA_LOG_ERROR);
				this->running = false;
				return;
			}

			break;
		}

		lastDispatched = std::chrono::steady_clock::now();;
		auto invoked = std::thread(connectionHandler, this);
		handlerDispatched = false;
		invoked.detach();
	}
}
