/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include "../include/lambda/httpcore.hpp"
#include "../include/lambda/lambda.hpp"
#include "../include/lambda/compress.hpp"
#include "../include/lambda/util.hpp"


lambda::lambda::lambda() {
	memset(&wsaData, 0, sizeof(wsaData));
	ListenSocket = INVALID_SOCKET;
	handlerDispatched = true;
	running = false;
	instanceWormhole = nullptr;
}

lambda::lambda::~lambda() {
	stop();
}

void lambda::lambda::setConfig(Config config) {
	instanceConfig = config;
}

void lambda::lambda::openWormhole(void* object) {
	instanceWormhole = object;
}

void lambda::lambda::closeWormhole() {
	instanceWormhole = nullptr;
}


std::string lambda::lambda::serverTime(time_t timestamp) {
	char timebuff[16];
	auto timedata = gmtime(&timestamp);
	strftime(timebuff, sizeof(timebuff), "%H:%M:%S", timedata);
	return std::string(timebuff);
}

std::string lambda::lambda::serverTime() {
	return serverTime(time(nullptr));
}


void lambda::lambda::addLogEntry(Context context, int typeCode, std::string message) {
	
	LogEntry entry;
		entry.type = typeCode;
		entry.message = message;
		entry.timestamp = time(nullptr);
		entry.requestId = context.requestId.substr(0, context.requestId.find_first_of('-'));
		entry.clientIP = context.clientIP;

	instanceLog.push_back(entry);
}

std::vector <std::string> lambda::lambda::showLogs() {

	std::vector <std::string> printout;

	std::lock_guard<std::mutex> lock (threadLock);

	for (auto& logEntry : instanceLog) {

		auto temp = serverTime();

		switch (logEntry.type) {

			case LAMBDA_LOG_WARN:
				temp += " [WARN] ";
			break;

			case LAMBDA_LOG_ERR:
				temp += " [ERRR] ";
			break;
			
			default:
				temp += " [INFO] ";
			break;
		}
		
		printout.push_back(temp + logEntry.clientIP + ' ' + logEntry.requestId + " : " + logEntry.message);
	}

	instanceLog.clear();

	return printout;
}

lambda::actionResult lambda::lambda::start(const int port, std::function<Response(Event)> lambdaCallbackFunction) {

	if (running) return {
		false,
		"Already running"
	};

	if (!instanceConfig.mutlipeInstances || (instanceConfig.mutlipeInstances && !socketsReady())) {
		if (WSAStartup(MAKEWORD(2,2), &wsaData)) return {
			false,
			"Startup failed",
			"Code:" + std::to_string(GetLastError())
		};
	}

	//	resolve server address
	struct addrinfo *servAddr = NULL;
	struct addrinfo addrHints;
		ZeroMemory(&addrHints, sizeof(addrHints));
		addrHints.ai_family = AF_INET;
		addrHints.ai_socktype = SOCK_STREAM;
		addrHints.ai_protocol = IPPROTO_TCP;
		addrHints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, std::to_string(port).c_str(), &addrHints, &servAddr) != 0) {
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		freeaddrinfo(servAddr);
		return {
			false,
			"Localhost didn't resolve",
			"Code:" + std::to_string(GetLastError())
		};
	}

	// create and bind a SOCKET
	ListenSocket = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		freeaddrinfo(servAddr);
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to create listening socket",
			"Code:" + std::to_string(GetLastError())
		};
	}

	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to bind a TCP socket",
			"Code:" + std::to_string(GetLastError())
		};
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Socket error",
			"Code:" + std::to_string(GetLastError())
		};
	}

	//	start watchdog
	requestCallback = lambdaCallbackFunction;
	running = true;
	worker = std::thread(connectDispatch, this);

	return {
		true,
		"Started"
	};
}

void lambda::lambda::stop() {
	running = false;
	if (worker.joinable()) worker.join();
	shutdown(ListenSocket, SD_BOTH);
	closesocket(ListenSocket);
	if (!instanceConfig.mutlipeInstances) WSACleanup();
}

void lambda::lambda::connectDispatch() {

	time_t lastDispatched = 0;

	while (running) {

		if(handlerDispatched) {

			auto invoked = std::thread(handler, this);
			handlerDispatched = false;
			lastDispatched = timeGetTime();
			invoked.detach();
			
		} else if (timeGetTime() > (lastDispatched + LAMBDA_DSP_SLEEP)) Sleep(LAMBDA_DSP_SLEEP);
	}
}

bool lambda::socketsReady() {
	bool result = true;

    SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (temp == INVALID_SOCKET){
		if (GetLastError() == WSANOTINITIALISED) result = false;
    }

    closesocket(temp);

	return result;
}

lambda::httpRequest lambda::socketGetHTTP(SOCKET* client) {

	//	receive http header first
	std::string rawData;
	bool opresult = true;
	auto headerEnded = std::string::npos;
	auto headerChunk = new char [LAMBDAHTTP_HEADER_CHUNK];

	while (headerEnded == std::string::npos && opresult) {

		auto bytesReceived = recv(*client, headerChunk, LAMBDAHTTP_HEADER_CHUNK, 0);
		if (bytesReceived <= 0) {
			//	drop the connection if error occured
			if (bytesReceived < 0) opresult = false;
			break;
		}
		if (!opresult) break;

		rawData.append(headerChunk, bytesReceived);

		//	"\r\n\r\n" - is a marker of http header end
		headerEnded = rawData.find("\r\n\r\n");
	}
	
	delete[] headerChunk;
	
	if (!opresult) return { false };

	std::string requestHeaderText = rawData.substr(0, headerEnded);

	//	split text by lines
	auto headerLines = splitBy(requestHeaderText, "\r\n");
	if (headerLines.size() < 1) return { false };

	//	parse start-line
	auto startArgs = splitBy(headerLines[0], " ");
	//	drop if less than 3 args here
	if (startArgs.size() < 3) return { false };

	httpHeaders headers;
	headerLines.erase(headerLines.begin());
	headers.parse(headerLines);

	//	download body if exists
	std::string requestBody;
	auto contentLength = headers.get("Content-Length");
	if (contentLength.size()) {
		size_t bodySize;

		try { bodySize = std::stoi(contentLength); }
			catch(...) { bodySize = 0; }

		if (bodySize) {
			
			requestBody = rawData.substr(headerEnded + 4);

			if (requestBody.size() < bodySize) {

				auto bodyChunk = new char [LAMBDAHTTP_BODY_CHUNK];
				while (requestBody.size() < bodySize) {
					
					auto bytesReceived = recv(*client, bodyChunk, LAMBDAHTTP_BODY_CHUNK, 0);

					if (bytesReceived <= 0) {
						if (bytesReceived < 0) opresult = false;
						break;
					}
					
					requestBody.append(bodyChunk, bytesReceived);
				}

				delete[] bodyChunk;
			}
		}
	}

    return {
		true,
		startArgs,
		headers,
		requestBody
	};
}

lambda::actionResult lambda::socketSendHTTP(SOCKET* client, std::string startline, httpHeaders& headers, const std::string& body) {

	//	create response message
	auto temp = startline + "\r\n";

	//	add content length header
	if (body.size()) headers.set("Content-Length", std::to_string(body.size()));

	//	add headers
	temp += headers.dump();
	
	//	end headers block
	temp += "\r\n";

	//	append body
	if (body.size()) temp.append(body);

	auto sendResult = send(*client, &temp[0], temp.size(), 0);

	//	success
	if (sendResult > 0) return {
		true,
		"Sent"
	};

	//	encountered an error
	return {
		false,
		"Network error",
		"WINAPI:" + std::to_string(GetLastError())
	};
}

void lambda::lambda::handler() {

	//	accept socket and free the flag for next handler instance
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	handlerDispatched = true;

	//	set socket timeouts
	const uint32_t tcpTimeout = LAMBDA_TCP_TIMEOUT_MS;
	setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tcpTimeout, sizeof(uint32_t));
	setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tcpTimeout, sizeof(uint32_t));	

	//	create metadata
	Context context;
		context.requestId = createUniqueId();
		context.started = time(nullptr);
		context.requestType = LAMBDA_REQ_LAMBDA;

	//	append client's ip to metadata
	char clientIPBuff[64];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)))
		context.clientIP = clientIPBuff;

	//	download http request
	auto incomingRequest = socketGetHTTP(&ClientSocket);
	//	drop connection if the request is invalid
	if (!incomingRequest.success) {
		addLogEntry(context, LAMBDA_LOG_WARN, "Aborted");
		shutdown(ClientSocket, SD_BOTH);
		closesocket(ClientSocket);
		return;
	}

	auto targetURL = toLowerCase(incomingRequest.arguments[1]);

	//	pass the data to the callback
	auto callbackResult = requestCallback({

		//	httpversion
		toUpperCase(incomingRequest.arguments[2]),
		//	requestID
		context.requestId,
		//	clientIP
		context.clientIP,

		//	wormhole
		instanceWormhole,

		//method
		toUpperCase(incomingRequest.arguments[0]),
		//	path
		targetURL.find('?') ? targetURL.substr(0, targetURL.find_last_of('?')) : targetURL,
		//	headers
		incomingRequest.headers,
		//	searchQuery
		httpSearchQuery(targetURL),
		//	body
		incomingRequest.body
	});

	auto responseHeaders = httpHeaders(callbackResult.headers);

	//	inject additional headers
	responseHeaders.add("X-Powered-By", LAMBDA_HTTP_USERAGENT);
	responseHeaders.add("X-Request-ID", context.requestId);
	responseHeaders.add("Date", httpTime());
	
	if (callbackResult.body.size()) {
		//	check if is a json object
		auto jsJsonObject = (callbackResult.body[0] == '{' && callbackResult.body[callbackResult.body.size() - 1] == '}');
		//	check if is a json array
		auto isJsonArray = (callbackResult.body[0] == '[' && callbackResult.body[callbackResult.body.size() - 1] == ']');
		//	apply content type
		responseHeaders.add("Content-Type", mimetype((jsJsonObject || isJsonArray) ? "json" : "html"));
	}

	//	apply request compression
	auto acceptEncodings = splitBy(incomingRequest.headers.get("Accept-Encoding"), ",");

	std::string compressedBody;
	static const std::vector <std::string> compressibleTypes = { "text", "application" };
	auto isCompressable = jstring(responseHeaders.get("Content-Type")).includes(compressibleTypes);
	
	if (instanceConfig.compression_enabled && acceptEncodings.size() && (isCompressable || instanceConfig.compression_allFileTypes)) {

		for (auto &&encoding : acceptEncodings) trimString(&encoding);

		if (instanceConfig.compression_preferBr) {
			for (auto encoding : acceptEncodings) {
				if (encoding == "br") {
					acceptEncodings[0] = encoding;
					break;
				}
			}
		}

		std::string appliedCompression;

		if (acceptEncodings.front() == "br") {
			compressedBody = brCompress(&callbackResult.body);
			if (compressedBody.size()) appliedCompression = "br";
				else addLogEntry(context, LAMBDA_LOG_ERR, "brotli compression failed");
			
		} else if (acceptEncodings.front() == "gzip") {
			compressedBody = gzCompress(&callbackResult.body, true);
			if (compressedBody.size()) appliedCompression = "gzip";
				else addLogEntry(context, LAMBDA_LOG_ERR, "gzip compression failed");

		} else if (acceptEncodings.front() == "deflate") {
			compressedBody = gzCompress(&callbackResult.body, false);
			if (compressedBody.size()) appliedCompression = "deflate";
				else addLogEntry(context, LAMBDA_LOG_ERR, "deflate compression failed");
		}

		if (appliedCompression.size()) responseHeaders.set("Content-Encoding", appliedCompression);
			else compressedBody.clear();
	}

	//	generate response title
	std::string startLine = "HTTP/1.1 " + httpStatusString(callbackResult.statusCode);

	//	send response and close socket
	auto responseSent = socketSendHTTP(&ClientSocket, startLine, responseHeaders, compressedBody.size() ? compressedBody : callbackResult.body);
	shutdown(ClientSocket, SD_BOTH);
	closesocket(ClientSocket);

	if (responseSent.success) addLogEntry(context, LAMBDA_LOG_INFO, "Resp. " + std::to_string(callbackResult.statusCode) + " for " + targetURL + "");
		else addLogEntry(context, LAMBDA_LOG_INFO, "Request for " + targetURL + " failed: " + responseSent.cause);

	//	done!
	return;
}
