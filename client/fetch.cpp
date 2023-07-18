#include "./client.hpp"
#include "../network/sysnetw.hpp"
#include "../network/network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Response Client::fetch(const Request& userRequest) {

	try {

		auto isWww = !(userRequest.url.pathname == "localhost" || userRequest.url.pathname == "127.0.0.01") && userRequest.url.pathname.starts_with("http");

		auto connection = isWww ? HTTPConnection(userRequest.url) : HTTPConnection((uint16_t)std::stoi(userRequest.url.port));
		
		//	complete http request
		auto request = userRequest;
		if (isWww) request.headers.set("Host", request.url.host);
		request.headers.append("User-Agent", LAMBDA_USERAGENT);
		request.headers.append("Accept-Encoding", LAMBDA_FETCH_ENCODINGS);
		request.headers.append("Accept", "*/*");
		request.headers.append("Connection", "close");

		connection.sendRequest(request);

		auto requestStream = request.dump();

		return connection.receiveResponse();

	} catch(const std::exception& error) {
		throw Lambda::Error("Fetch failed", error);
	}
}

HTTP::Response Client::fetch(std::string url) {
	auto request = Request();
	request.url.setHref(url);
	return fetch(request);
}
