#include "./server.hpp"
#include "../http/http.hpp"
#include <chrono>

Lambda::Server::Server() {

	this->ListenSocketObj = new HTTPSocket::ListenSocket();
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
		addLogRecord(std::string("Request aborted, socket error: ") + "client.ip");
		return;
	}

	auto request = client.receiveMessage();

	puts(request.path().c_str());

	auto resp = HTTP::Response();

	resp.headers.fromEntries({{"Server", "maddsua/lambda"}});
	
	client.sendMessage(resp);
}
