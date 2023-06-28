#include "./server.hpp"
#include "../http/http.hpp"
#include "../lambda.hpp"
#include <chrono>

using namespace Lambda;

Server::Server() {

	this->ListenSocketObj = new Socket::HTTPListenSocket();

	if (!ListenSocketObj->ok()) {
		auto sockstat = ListenSocketObj->status();
		addLogRecord("Failed to start server: Socket error:" + std::to_string(sockstat.code), LAMBDA_LOG_ERROR);
		return;
	}
	
	running = true;
	handlerDispatched = true;
	watchdogThread = new std::thread(connectionWatchdog, this);
	addLogRecord("Server start successful", LAMBDA_LOG_INFO);
}

Server::~Server() {
	running = false;
	if (watchdogThread->joinable())
		watchdogThread->join();
	delete ListenSocketObj;
	delete watchdogThread;
}

void Server::setServerCallback(void (*callback)(HTTP::Request&, Context&)) {
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

void Server::setPasstrough(void* object) {
	this->instancePasstrough = object;
}
void Server::removePasstrough() {
	this->instancePasstrough = nullptr;
}

void Server::connectionWatchdog() {

	auto lastDispatched = std::chrono::system_clock::now();

	while (running) {

		if (!handlerDispatched) {

			if ((std::chrono::system_clock::now() - lastDispatched) > std::chrono::milliseconds(1)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				//puts(std::to_string((std::chrono::system_clock::now() - lastDispatched).count()).c_str());
			}

			continue;
		}

		if (!ListenSocketObj->ok()) {
			auto status = ListenSocketObj->status();
			
			addLogRecord("Listen socket failed, code: " + std::to_string(status.code) + " Restarting...", LAMBDA_LOG_WARN);

			delete ListenSocketObj;
			ListenSocketObj = new Socket::HTTPListenSocket();
			status = ListenSocketObj->status();

			if (!ListenSocketObj->ok()) {
				this->running = false;
				addLogRecord("Failed to restart listen socket. Code " + std::to_string(status.code) + " Aborting.", LAMBDA_LOG_ERROR);
				break;
			}
		}

		lastDispatched = std::chrono::system_clock::now();
		auto invoked = std::thread(connectionHandler, this);
		handlerDispatched = false;
		invoked.detach();
	}
}

void Server::connectionHandler() {
	
	auto client = ListenSocketObj->acceptConnection();
	handlerDispatched = true;

	if (!client.ok()) {
		addLogRecord(std::string("Request aborted, socket error on client: ") + client.ip());
		return;
	}

	// get request payload and context
	auto request = client.receiveMessage();
	Context requestCTX;
	requestCTX.clientIP = client.ip();
	requestCTX.passtrough = this->instancePasstrough;

	//	serverfull handler. note the return statement
	if (this->requestCallback != nullptr) {
		this->requestCallback(request, requestCTX);
		return;
	}

	auto response = HTTP::Response();

	//	serverless handler
	if (this->requestCallbackServerless != nullptr) {
		response = (*requestCallbackServerless)(request, requestCTX);
	}
	//	fallback handler
	else {
		addLogRecord(std::string("Request handled in fallback mode. Path: " ) + request.path + ", client: " + client.ip());
		response.setBodyText(std::string("server works. lambda v") + LAMBDAVERSION);
	}

	//	set some service headers
	response.headers.append("server", "maddsua/lambda");
	response.headers.append("date", HTTP::serverDate());
	response.headers.append("content-type", "text/plain");

	client.sendMessage(response);
}
