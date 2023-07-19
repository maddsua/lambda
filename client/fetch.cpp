#include "./client.hpp"
#include "../network/sysnetw.hpp"
#include "../network/network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Response Client::fetch(const Request& userRequest) {

	try {

		if (userRequest.url.protocol != "http")
			throw Lambda::Error(std::string("Only http/1.1 is supported by fetch API in version ") + LAMBDA_VERSION);

		auto connection = HTTPConnection(userRequest.url);
		auto isWww = userRequest.url.isWWW();
		
		//	complete http request
		auto request = userRequest;
		if (isWww) request.headers.set("Host", request.url.host);
		request.headers.append("User-Agent", LAMBDA_USERAGENT);
		request.headers.append("Accept-Encoding", LAMBDA_FETCH_ENCODINGS);
		request.headers.append("Accept", "*/*");
		request.headers.append("Connection", "close");

		connection.sendRequest(request);

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
