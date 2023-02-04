#include "../include/maddsua/http.hpp"

lambda::httpRequest lambda::socketGetHTTP(SOCKET* client) {

	//	receive http header first
	std::string rawData;
	bool opresult = true;
	auto headerEnded = std::string::npos;
	auto headerChunk = new char [HTTPLAMBDA_HEADER_CHUNK];

	while (headerEnded == std::string::npos && opresult) {

		auto bytesReceived = recv(*client, headerChunk, HTTPLAMBDA_HEADER_CHUNK, 0);
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
	
	delete headerChunk;
	if (!opresult) return { false };

	std::string requestHeaderText = rawData.substr(0, headerEnded);

	//	split text by lines
	auto headerLines = splitBy(requestHeaderText.c_str(), "\r\n");
	if (headerLines.size() < 1) return { false };

	//	parse start-line
	std::vector <std::string> startArgs = splitBy(headerLines[0].c_str(), " ");
	if (startArgs.size() < 3) return { false };

	//	parse headers
	std::vector <lambda::datapair> headers;
	for (size_t i = 1; i < headerLines.size(); i++) {
		auto hLine = headerLines[i];
		auto delim = hLine.find_first_of(':');

		if (delim == std::string::npos) break;

		headers.push_back({
			toLowerCase(trim(hLine.substr(0, delim))),
			trim(hLine.substr(delim + 1))
		});
	}

	//	download body is exists
	std::string requestBody;
	auto contentLength = headerFind("Content-Length", &headers);
	if (contentLength.size()) {
		size_t bodySize;

		try { bodySize = std::stoi(contentLength); }
		catch(...) { bodySize = 0; }

		if (bodySize) {
			
			requestBody = rawData.substr(headerEnded + 4);
			const size_t remains = (requestBody.size() < bodySize) ? (bodySize - requestBody.size()) : 0;

			if (remains) {

				auto bodyChunk = new char [HTTPLAMBDA_BODY_CHUNK];
				while (requestBody.size() < bodySize) {
					
					auto bytesReceived = recv(*client, bodyChunk, HTTPLAMBDA_BODY_CHUNK, 0);

					if (bytesReceived <= 0) {
						if (bytesReceived < 0) opresult = false;
						break;
					}
					
					requestBody.append(bodyChunk, bytesReceived);
				}

				delete bodyChunk;
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

lambda::actionResult lambda::socketSendHTTP(SOCKET* client, std::string startline, std::vector <datapair>* headers, std::string* body) {

	//	create reaponse message
	auto temp = startline + "\r\n";

	//	add content length header
	if (body->size()) headerInsert("Content-Length", std::to_string(body->size()), headers);

	//	add headers
	for (auto header : *headers) {
		temp += header.name + ": " + header.value + "\r\n";
	}
	
	//	end headers block
	temp += "\r\n";

	//	append body
	if (body->size()) {
		temp.append(*body);
	}

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