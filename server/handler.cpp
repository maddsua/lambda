#include "../lambda.hpp"
#include "./server.hpp"
#include "../http/http.hpp"
#include <chrono>

using namespace Lambda;
using namespace Lambda::Network;

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
