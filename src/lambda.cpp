#include "../include/maddsua/lambda.hpp"


void lambda::lambda::addLogEntry(std::string type, std::string text) {
	
	auto servertime = []() {
		char timebuff[16];
		auto now = time(nullptr);
		tm timedata = *gmtime(&now);
		strftime(timebuff, sizeof(timebuff), "%H:%M:%S", &timedata);
		return std::string(timebuff);
	} ();

	serverlog.push_back(toUpperCase(type) + " [" + servertime + "] " + text);
}


lambda::actionResult lambda::lambda::init(const uint32_t port, std::function<lambdaResponse(lambdaEvent)> lambda) {

	if (running) return {
		false,
		"Already running"
	};

	if (!config.mutlipeInstances || (config.mutlipeInstances && !socketsReady())) {
		if (WSAStartup(MAKEWORD(2,2), &wsaData)) return {
			false,
			"Startup failed",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	//	resolve server address
	struct addrinfo *servAddr = NULL;
	struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &servAddr) != 0) {
		if (!config.mutlipeInstances) WSACleanup();
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
		if (!config.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to create listening socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}
	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		if (!config.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to bind a TCP socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		if (!config.mutlipeInstances) WSACleanup();
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

void lambda::lambda::close() {
	running = false;
	if (worker.joinable()) worker.join();
	closesocket(ListenSocket);
	if (!config.mutlipeInstances) WSACleanup();
}

void lambda::lambda::connectManager() {

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

void lambda::lambda::handler() {

	//	accept socket and free the flag for next handler instance
	SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
	handlerDispatched = true;

	//	download http request
	auto rqData = socketGetHTTP(&ClientSocket);

	//	drop connection if the request is invalid
	if (!rqData.success) {
		addLogEntry("Error", "Invalid request or connection problem");
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
	headerAdd({"X-Powered-By", MADDSUAHTTP_USERAGENT}, &lambdaResult.headers);
	headerAdd({"Date", httpTimeNow()}, &lambdaResult.headers);
	headerAdd({"Content-Type", findMimeType("html")}, &lambdaResult.headers);

	//	generate response title
	std::string startLine = "HTTP/1.1 " + httpStatusString(lambdaResult.statusCode);

	//	apply compression
	auto acceptEncodings = splitBy(headerFind("Accept-Encoding", &rqData.headers), ",");

	auto isCompressable = includes(headerFind("Content-Type",  &lambdaResult.headers), compressableTypes);
	std::string compressedBody;
	
	if (config.compression_enabled && acceptEncodings.size() && (isCompressable || config.compression_allFileTypes)) {

		for (auto &&encoding : acceptEncodings) {
			trim(&encoding);
		}

		if (config.compression_preferBr) {
			for (auto encoding : acceptEncodings) {
				if (encoding == "br") {
					acceptEncodings[0] = encoding;
					break;
				}
			}
		}

		std::string appliedCompression;

		if (acceptEncodings[0] == "br") {

			if (compression::brCompress(&lambdaResult.body, &compressedBody)) appliedCompression = "br";
				else addLogEntry("Error", "brotli compression failed");
			
		} else if (acceptEncodings[0] == "gzip") {

			if (compression::gzCompress(&lambdaResult.body, &compressedBody, true)) appliedCompression = "gzip";
				else addLogEntry("Error", "gzip compression failed");

		} else if (acceptEncodings[0] == "deflate") {
			
			if (compression::gzCompress(&lambdaResult.body, &compressedBody, false)) appliedCompression = "deflate";
				else addLogEntry("Error", "deflate compression failed");
		}

		if (appliedCompression.size()) headerInsert("Content-Encoding", appliedCompression, &lambdaResult.headers);
			else compressedBody.erase(compressedBody.begin(), compressedBody.end());
	}


	//	send response and close socket
	auto sent = socketSendHTTP(&ClientSocket, startLine, &lambdaResult.headers, compressedBody.size() ? &compressedBody : &lambdaResult.body);
	closesocket(ClientSocket);

	if (sent.success) {
		addLogEntry("Info", "Response with status " + std::to_string(lambdaResult.statusCode) + " for \"" + rqEvent.path + "\"");

	} else {
		addLogEntry("Info", "Request for \"" + rqEvent.path + "\" failed: " + sent.cause);
	}

	//	done!
}