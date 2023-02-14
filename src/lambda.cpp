#include "../include/maddsua/lambda.hpp"


const std::vector<std::string> compressibleTypes = { "text", "application" };

void lambda::lambda::setConfig(lambdaConfig config) {
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


void lambda::lambda::addLogEntry(lambdaInvokContext context, short typeCode, std::string message) {
	
	lambdaLogEntry entry;
		entry.type = typeCode;
		entry.message = message;
		entry.timestamp = time(nullptr);
		entry.requestId = context.uuid;
		entry.clientIP = context.clientIP;

	instanceLog.push_back(entry);
}

std::vector <std::string> lambda::lambda::showLogs() {

	std::vector <std::string> printout;

	std::lock_guard<std::mutex> lock (threadLock);

	for (auto& logEntry : instanceLog) {

		auto temp = serverTime();

		switch (logEntry.type) {
			case LAMBDALOG_WARN:
				temp += " [WARN] ";
			break;

			case LAMBDALOG_ERR:
				temp += " [ERRR] ";
			break;
			
			default:
				temp += " [INFO] ";
			break;
		}
		
		printout.push_back(temp + logEntry.clientIP + ' ' + formatUUID(logEntry.requestId, false) + " : " + logEntry.message);
	}

	instanceLog.clear();

	return printout;
}


lambda::actionResult lambda::lambda::start(const int port, std::function<lambdaResponse(lambdaEvent)> lambda) {

	if (running) return {
		false,
		"Already running"
	};

	if (!instanceConfig.mutlipeInstances || (instanceConfig.mutlipeInstances && !socketsReady())) {
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
		if (!instanceConfig.mutlipeInstances) WSACleanup();
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
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to create listening socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}
	if (bind(ListenSocket, servAddr->ai_addr, (int)servAddr->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(servAddr);
		closesocket(ListenSocket);
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Failed to bind a TCP socket",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	freeaddrinfo(servAddr);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(ListenSocket);
		if (!instanceConfig.mutlipeInstances) WSACleanup();
		return {
			false,
			"Socket error",
			"WINAPI:" + std::to_string(GetLastError())
		};
	}

	//	start watchdog
	callback = lambda;
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

void lambda::lambda::handler() {

	//	accept socket and free the flag for next handler instance
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
	handlerDispatched = true;

	//	create metadata
	lambdaInvokContext context;
		context.uuid = createByteUUID();
		context.started = time(nullptr);
		context.requestType = LAMBDAREQ_LAMBDA;

	//	append client's ip to metadata
	char clientIPBuff[50];
	if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIPBuff, sizeof(clientIPBuff)))
		context.clientIP = clientIPBuff;

	//	download http request
	auto rqData = socketGetHTTP(&ClientSocket);

	//	drop connection if the request is invalid
	if (!rqData.success) {
		addLogEntry(context, LAMBDALOG_WARN, "Aborted");
		closesocket(ClientSocket);
		return;
	}

	//	add client's useragent to metadata
	//auto clientUA = headerFind("User-Agent", &rqData.headers);
	//if (clientUA.size()) context.userAgent = clientUA;


	//	pass the data to lambda function
	auto targetURL = rqData.startLineArgs[1];
	lambdaEvent rqEvent;
		rqEvent.method = rqData.startLineArgs[0];
		rqEvent.httpversion = rqData.startLineArgs[2];
		rqEvent.path = targetURL.find('?') ? targetURL.substr(0, targetURL.find_last_of('?')) : targetURL;
		rqEvent.searchQuery = searchQueryParams(&targetURL);
		rqEvent.headers = rqData.headers;
		rqEvent.body = rqData.body;

		//	neutron-star-explosive part
		rqEvent.wormhole = instanceWormhole;
		
	auto lambdaResult = callback(rqEvent);

	//	inject additional headers
	headerAdd({"X-Powered-By", MADDSUAHTTP_USERAGENT}, &lambdaResult.headers);
	headerAdd({"X-Request-ID", formatUUID(context.uuid, true)}, &lambdaResult.headers);
	headerAdd({"Date", httpTimeNow()}, &lambdaResult.headers);
	headerAdd({"Content-Type", findMimeType("html")}, &lambdaResult.headers);

	//	reset header case
	for (size_t i = 0; i < lambdaResult.headers.size(); i++)
		toTitleCase(&lambdaResult.headers[i].name);

	//	apply request compression
	auto acceptEncodings = splitBy(headerFind("Accept-Encoding", &rqData.headers), ",");

	auto isCompressable = includes(headerFind("Content-Type",  &lambdaResult.headers), compressibleTypes);
	std::string compressedBody;
	
	if (instanceConfig.compression_enabled && acceptEncodings.size() && (isCompressable || instanceConfig.compression_allFileTypes)) {

		for (auto &&encoding : acceptEncodings) {
			trim(&encoding);
		}

		if (instanceConfig.compression_preferBr) {
			for (auto encoding : acceptEncodings) {
				if (encoding == "br") {
					acceptEncodings[0] = encoding;
					break;
				}
			}
		}

		std::string appliedCompression;

		if (acceptEncodings[0] == "br") {
			compressedBody = compression::brCompress(&lambdaResult.body);
			if (compressedBody.size()) appliedCompression = "br";
				else addLogEntry(context, LAMBDALOG_ERR, "brotli compression failed");
			
		} else if (acceptEncodings[0] == "gzip") {
			compressedBody = compression::gzCompress(&lambdaResult.body, true);
			if (compressedBody.size()) appliedCompression = "gzip";
				else addLogEntry(context, LAMBDALOG_ERR, "gzip compression failed");

		} else if (acceptEncodings[0] == "deflate") {
			compressedBody = compression::gzCompress(&lambdaResult.body, false);
			if (compressedBody.size()) appliedCompression = "deflate";
				else addLogEntry(context, LAMBDALOG_ERR, "deflate compression failed");
		}

		if (appliedCompression.size()) headerInsert("Content-Encoding", appliedCompression, &lambdaResult.headers);
			else compressedBody.erase(compressedBody.begin(), compressedBody.end());
	}

	//	generate response title
	std::string startLine = "HTTP/1.1 " + httpStatusString(lambdaResult.statusCode);

	//	send response and close socket
	auto sent = socketSendHTTP(&ClientSocket, startLine, &lambdaResult.headers, compressedBody.size() ? &compressedBody : &lambdaResult.body);
	closesocket(ClientSocket);

	if (sent.success) {
		addLogEntry(context, LAMBDALOG_INFO, "Resp. " + std::to_string(lambdaResult.statusCode) + " for " + rqEvent.path + "");
	} else {
		addLogEntry(context, LAMBDALOG_INFO, "Request for " + rqEvent.path + " failed: " + sent.cause);
	}

	//	done!
	return;
}

