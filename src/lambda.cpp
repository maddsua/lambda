#include "../include/maddsua/lambda.hpp"
#include "../include/maddsua/compress.hpp"

void maddsua::lambda::addLogEntry(std::string module, std::string type, std::string text) {
	
	auto servertime = []() {
		char timebuff[16];
		auto now = time(nullptr);
		tm timedata = *gmtime(&now);
		strftime(timebuff, sizeof(timebuff), "%H:%M:%S", &timedata);
		return std::string(timebuff);
	} ();

	serverlog.push_back(std::string(module) + ": " + toUpperCase(type) + " [" + servertime + "] " + text);
}


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
	auto rqData = socketGetHTTP(&ClientSocket);

	//	drop connection if the request is invalid
	if (!rqData.success) {
		addLogEntry("Lambda", "Error", "Invalid request or connection problem");
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
	auto acceptEncodings = headerFind("Accept-Encoding", &rqData.headers);
	auto compressableTypes = std::vector<std::string>({
		"text",
		"application"
	});

	auto isCompressable = includes(headerFind("Content-Type",  &lambdaResult.headers), compressableTypes);
	std::string compressedBody;
	
	if (config.useCompression && (isCompressable || config.compressAll)) {

		std::string appliedCompression;

		if (includes(&acceptEncodings, "br")) {

			if (maddsua::brCompress(&lambdaResult.body, &compressedBody)) appliedCompression = "br";
				else addLogEntry("Lambda", "Error", "brotli compression failed");
			
		} else if (includes(&acceptEncodings, "gzip")) {

			if (maddsua::gzCompress(&lambdaResult.body, &compressedBody, true)) appliedCompression = "gzip";
				else addLogEntry("Lambda", "Error", "gzip compression failed");

		} else if (includes(&acceptEncodings, "deflate")) {
			
			if (maddsua::gzCompress(&lambdaResult.body, &compressedBody, false)) appliedCompression = "deflate";
				else addLogEntry("Lambda", "Error", "deflate compression failed");
		}

		if (appliedCompression.size()) headerInsert("Content-Encoding", appliedCompression, &lambdaResult.headers);
			else compressedBody.erase(compressedBody.begin(), compressedBody.end());
	}


	//	send response and close socket
	auto sent = socketSendHTTP(&ClientSocket, startLine, &lambdaResult.headers, compressedBody.size() ? &compressedBody : &lambdaResult.body);
	closesocket(ClientSocket);

	if (sent.success) {
		addLogEntry("Lambda", "Info", "Response with status " + std::to_string(lambdaResult.statusCode) + " for \"" + rqEvent.path + "\"");

	} else {
		addLogEntry("Lambda", "Info", "Request for \"" + rqEvent.path + "\" failed: " + sent.cause);
	}

	//	done!
}