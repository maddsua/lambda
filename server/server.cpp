#include "../lambda.hpp"
#include "./server.hpp"
#include "../http/http.hpp"
#include <chrono>

using namespace Lambda;
using namespace Lambda::Network;

Server::Server() {

	try {
		this->ListenSocketObj = new Network::ListenSocket();
	} catch(const std::exception& e) {
		addLogRecord("Failed to start server: Socket error:" + std::string(e.what()), LAMBDA_LOG_ERROR);
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

void Server::setServerCallback(void (*callback)(Network::HTTPConnection&, Context&)) {
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

		/*if (!ListenSocketObj->isAlive()) {
			auto status = ListenSocketObj->status();
			
			addLogRecord("Listen socket failed, code: " + std::to_string(status.code) + " Restarting...", LAMBDA_LOG_WARN);

			delete ListenSocketObj;
			ListenSocketObj = new Socket::ListengSocket();
			status = ListenSocketObj->status();

			if (!ListenSocketObj->ok()) {
				this->running = false;
				addLogRecord("Failed to restart listen socket. Code " + std::to_string(status.code) + " Aborting.", LAMBDA_LOG_ERROR);
				break;
			}
		}*/

		lastDispatched = std::chrono::system_clock::now();
		auto invoked = std::thread(connectionHandler, this);
		handlerDispatched = false;
		invoked.detach();
	}
}

void Server::connectionHandler() {

	Context requestCTX;

	try {

		auto client = ListenSocketObj->acceptConnection();
		handlerDispatched = true;

		// get request context
		requestCTX.clientIP = client.clientIP();
		requestCTX.passtrough = this->instancePasstrough;

		//	hold response object ready just in case
		auto response = HTTP::Response();
		std::string handlerErrorMessage;

		//	bigass trycatch in case someone throws something diffrernt from std::exception
		//	Sadly, this is not gonna help in case of "the C error" strike...I mean segfault
		//	zlib and other cool libs can still crash the server
		try {

			//	serverfull handler. note the return statement
			if (this->requestCallback != nullptr) {
				this->requestCallback(client, requestCTX);
				return;
			}

			//	serverless handler
			//	we read request before even trying to call handler, so we won't break http in case there's no handler
			auto request = client.receiveMessage();
			if (this->requestCallbackServerless != nullptr) {
				response = (*requestCallbackServerless)(request, requestCTX);
			} else {
				throw std::runtime_error("No handler function assigned");
			}
	
		} catch(const std::exception& e) {
			handlerErrorMessage = e.what();
		} catch(...) {
			handlerErrorMessage = "Unhandled callback error. This must be caused by your code, bc lambda only throws std::exception's, and it would be catched long before this stage";
		}

		//	handle errors
		if (handlerErrorMessage.size()) {
			addLogRecord(std::string("Request failed: " ) + handlerErrorMessage + " | Client: " + client.clientIP(), LAMBDA_LOG_ERROR);
			response.setStatusCode(500);
			response.setBodyText(handlerErrorMessage + " | lambda v" + LAMBDAVERSION);
		}

		//	set some service headers
		response.headers.append("server", "maddsua/lambda");
		response.headers.append("date", HTTP::serverDate());
		response.headers.append("content-type", "text/plain");

		client.sendMessage(response);

	} catch(const Lambda::Error& e) {
		addLogRecord(std::string("Request has been aborted: ") + e.what() + "; client: " + (requestCTX.clientIP.size() ? requestCTX.clientIP : "unknown"));
	}
}
