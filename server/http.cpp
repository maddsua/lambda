#include "./server.hpp"
#include "../http/http.hpp"
#include "../lambda/lambda.hpp"
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
		addLogRecord(std::string("Request aborted, socket error on client: ") + client.metadata());
		return;
	}

	auto request = client.receiveMessage();


	//	fallback handler
	addLogRecord(std::string("Request handled in fallback mode. Path: " ) + request.path() + ", client: " + client.metadata());
	auto resp = HTTP::Response();
	resp.headers.append("server", "maddsua/lambda");
	resp.headers.append("content-type", "text/plain");
	resp.headers.append("date", HTTP::serverDate());
	resp.setBodyText(std::string("server works. lambda v") + LAMBDAVERSION);
	client.sendMessage(resp);
}
