#include "./server.hpp"
#include "../http/http.hpp"
#include "../lambda.hpp"
#include <chrono>

Lambda::Server::Server() {

	this->ListenSocketObj = new LambdaSocket::HTTPListenSocket();
	if (!ListenSocketObj->ok()) {
		auto sockstat = ListenSocketObj->status();
		addLogRecord("Failed to start server: Socket error:" + std::to_string(sockstat.status));
		return;
	}
	//throw Lambda::exception("Failed to start server, code: " + std::to_string(ListenSocketObj->status().status) + "/" + std::to_string(ListenSocketObj->status().code));

	running = true;
	handlerDispatched = true;
	watchdogThread = new std::thread(connectionWatchdog, this);
	addLogRecord("Server start successful");
}

Lambda::Server::~Server() {
	running = false;
	if (watchdogThread->joinable())
		watchdogThread->join();
	delete ListenSocketObj;
	delete watchdogThread;
}

void Lambda::Server::setServerCallback(void (*callback)(HTTP::Request, Context)) {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallback = callback;
}
void Lambda::Server::removeServerCallback() {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallback = nullptr;
}
void Lambda::Server::setServerlessCallback(HTTP::Response (*callback)(HTTP::Request, Context)){
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallbackServerless = callback;
}
void Lambda::Server::removeServerlessCallback() {
	//std::lock_guard<std::mutex>lock(mtLock);
	this->requestCallbackServerless = nullptr;
}

void Lambda::Server::setPasstrough(void* object) {
	this->instancePasstrough = object;
}
void Lambda::Server::removePasstrough() {
	this->instancePasstrough = nullptr;
}

void Lambda::Server::connectionWatchdog() {

	auto lastDispatched = std::chrono::system_clock::now();

	while (running) {

		if (!handlerDispatched) {

			if ((std::chrono::system_clock::now() - lastDispatched) > std::chrono::milliseconds(1)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				//puts(std::to_string((std::chrono::system_clock::now() - lastDispatched).count()).c_str());
			}

			continue;
		}

		lastDispatched = std::chrono::system_clock::now();
		auto invoked = std::thread(connectionHandler, this);
		handlerDispatched = false;
		invoked.detach();
	}
}

void Lambda::Server::connectionHandler() {
	
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
	response.headers.set("server", "maddsua/lambda");
	response.headers.set("date", HTTP::serverDate());

	//	serverless handler
	if (this->requestCallbackServerless != nullptr) {
		response = (*requestCallbackServerless)(request, requestCTX);
	}

	//	fallback handler
	else {
		addLogRecord(std::string("Request handled in fallback mode. Path: " ) + request.path() + ", client: " + client.ip());
		response.headers.set("content-type", "text/plain");
		response.setBodyText(std::string("server works. lambda v") + LAMBDAVERSION);
	}

	client.sendMessage(response);
}
