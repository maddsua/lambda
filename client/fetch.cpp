#include "./client.hpp"
#include "../network/tcpip.hpp"
#include "../network/network.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Response Client::fetch(const Request& userRequest) {

	SOCKET connection;

	try {
		connection = Network::resolveAndConnect(userRequest.url.host.c_str(), userRequest.url.port.c_str());
	} catch(const Lambda::Error& e) {
		throw Lambda::Error(std::string("Failed to resolve host: ") + e.what());
	}

	uint32_t connectionTimeout = 30000;
	auto setOptStatRX = setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (const char*)&connectionTimeout, sizeof(connectionTimeout));
	auto setOptStatTX = setsockopt(connection, SOL_SOCKET, SO_SNDTIMEO, (const char*)&connectionTimeout, sizeof(connectionTimeout));
	if (setOptStatRX != 0 || setOptStatTX != 0) {
		auto errcode = getAPIError();
		throw Lambda::Error("HTTP connection aborted: failed to set socket timeouts", errcode);
	}

	//	complete http request
	auto request = userRequest;
	request.headers.set("Host", /*"http://" + */request.url.host);
	request.headers.append("User-Agent", LAMBDA_USERAGENT);
	request.headers.append("Accept-Encoding", LAMBDA_FETCH_ENCODINGS);
	request.headers.append("Accept", "*/*");
	request.headers.append("Connection", "close");

	auto requestStream = request.dump();

	//	send request
	if (send(connection, (const char*)requestStream.data(), requestStream.size(), 0) <= 0) {
		auto apierror = getAPIError();
		closesocket(connection);
		throw Lambda::Error("Failed to perform http request", apierror);
	}
	
	//	get response
	auto response = Network::receiveHTTPResponse(connection);

	//	cleanup
	closesocket(connection);

	return response;
}

HTTP::Response Client::fetch(std::string url) {
	auto request = Request();
	request.url.setHref(url);
	return fetch(request);
}
