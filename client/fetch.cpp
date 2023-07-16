#include "./client.hpp"
#include "../network/tcpip.hpp"
#include "../network/network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Network;

Response Client::fetch(const Request& userRequest) {

	HTTPConnection connection;

	try {
		connection = std::move(HTTPConnection(userRequest.url));
	} catch(const Lambda::Error& e) {
		throw Lambda::Error(std::string("Fetch failed: ") + e.what());
	}

	//	complete http request
	auto request = userRequest;
	request.headers.set("Host", /*"http://" + */request.url.host);
	request.headers.append("User-Agent", LAMBDA_USERAGENT);
	request.headers.append("Accept-Encoding", LAMBDA_FETCH_ENCODINGS);
	request.headers.append("Accept", "*/*");
	request.headers.append("Connection", "close");

	auto rqSendResult = connection.sendRequest(request);
	if (rqSendResult.isError()) throw rqSendResult;

	auto requestStream = request.dump();

	return connection.receiveResponse();
}

HTTP::Response Client::fetch(std::string url) {
	auto request = Request();
	request.url.setHref(url);
	return fetch(request);
}
