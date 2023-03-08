#include "../include/maddsua/lambda.hpp"
#include "../include/maddsua/compress.hpp"


maddsua::actionResult maddsua::lambda::init(const char* port, std::function<lambdaResponse(lambdaEvent)> lambda) {

	if (running) return {
		false,
		"Already running"
	};

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) return {
		false,
		"Startup failed",
		"WINAPI:" + std::to_string(GetLastError())
	};

	//	resolve server address
	struct addrinfo *servAddr = NULL;
	struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, port, &hints, &servAddr) != 0) {
		WSACleanup();
		return {
			false,
			"Localhost didn't resolve",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	// create and bind a SOCKET
	ListenSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(servAddr);
		WSACleanup();
		return {
			false,
			"Failed to create listening socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}
	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		WSACleanup();
		return {
			false,
			"Failed to bind a TCP socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		WSACleanup();
		return {
			false,
			"Socket error",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	//	start watchdog
	callback = lambda;
	running = true;
	worker = std::thread(connectManager, this);

	return {
		true,
		"Started"
	};
}

void maddsua::lambda::close() {
	running = false;
	if (worker.joinable()) worker.join();
	closesocket(ListenSocket);
	WSACleanup();
}

void maddsua::lambda::connectManager() {

	while (running) {

		if(handlerDispatched) {
			auto invoked = std::thread(handler, this);
			handlerDispatched = false;
			invoked.detach();
			
		} else {
			Sleep(10);
		}
	}
}

void maddsua::lambda::handler() {

	//	accept socket and free the flag for next handler instance
	SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
	handlerDispatched = true;

	//	download http request
	auto rqData = _getData(&ClientSocket);

	//	drop connection if the request is invalid
	if (rqData.startLineArgs.size() < 3) {
		serverlog.push_back({ "Handler", "Now", "Connection dropped" });
		closesocket(ClientSocket);
		return;
	}

	auto targetURL = rqData.startLineArgs[1];

	//	pass the data to lambda function
	lambdaEvent rqEvent;
		rqEvent.method = rqData.startLineArgs[0];
		rqEvent.httpversion = rqData.startLineArgs[2];
		rqEvent.path = targetURL.find('?') ? targetURL.substr(0, targetURL.find_last_of('?')) : targetURL;
		rqEvent.searchQuery = searchQueryParams(&targetURL);
		rqEvent.headers = rqData.headers;
		rqEvent.body = rqData.body;
		
	auto lambdaResult = callback(rqEvent);

	//	reset header case
	for (size_t i = 0; i < lambdaResult.headers.size(); i++) {
		toTitleCase(&lambdaResult.headers[i].name);
	}

	//	inject additional headers
	if (!findHeader("X-Powered-By", &lambdaResult.headers).size()) lambdaResult.headers.push_back({"X-Powered-By", "maddsua/lambda"});
	if (!findHeader("Date", &lambdaResult.headers).size()) lambdaResult.headers.push_back({"Date", formattedTime(time(nullptr))});
	if (!findHeader("Content-Type", &lambdaResult.headers).size()) lambdaResult.headers.push_back({"Content-Type", findMimeType("html")});


	//	generate response title
	std::string startLine = "HTTP/1.1 ";
	auto statText = _findHttpCode(lambdaResult.statusCode);
	startLine += (statText.size() ? (std::to_string(lambdaResult.statusCode) + " " + statText) : "200 OK");

	//	apply compression
	auto acceptEncodings = findHeader("Accept-Encoding", &rqData.headers);
	std::string compressedBody;
	if (config_useCompression) {

		if (acceptEncodings.find("br") != std::string::npos) {

			if (!maddsua::brCompress(&lambdaResult.body, &compressedBody)) {
				compressedBody.erase(compressedBody.begin(), compressedBody.end());
				serverlog.push_back({ "Handler", "Now", "Brotli compression failed" });
			} else lambdaResult.headers.push_back({"Content-Encoding", "br"});
			
		} else if (acceptEncodings.find("gzip") != std::string::npos) {

			if (!maddsua::gzCompress(&lambdaResult.body, &compressedBody, true)) {
				compressedBody.erase(compressedBody.begin(), compressedBody.end());
				serverlog.push_back({ "Handler", "Now", "gzip compression failed" });
			} else lambdaResult.headers.push_back({"Content-Encoding", "gzip"});

		} else if (acceptEncodings.find("deflate") != std::string::npos) {
			
			if (!maddsua::gzCompress(&lambdaResult.body, &compressedBody, false)) {
				compressedBody.erase(compressedBody.begin(), compressedBody.end());
				serverlog.push_back({ "Handler", "Now", "deflate compression failed" });
			} else lambdaResult.headers.push_back({"Content-Encoding", "deflate"});
		}
	}


	//	send response and close socket
	auto sent = _sendData(&ClientSocket, startLine, &lambdaResult.headers, compressedBody.size() ? &compressedBody : &lambdaResult.body);
	closesocket(ClientSocket);

	if (sent.success) serverlog.push_back({ "Handler", "Now", "Response with status " + std::to_string(lambdaResult.statusCode) + " for \"" + rqEvent.path + "\"" });
		else serverlog.push_back({ "Handler", "Now", "Request for \"" + rqEvent.path + "\" failed: " + sent.cause });
	//	done!
}