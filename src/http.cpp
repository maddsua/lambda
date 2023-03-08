#include "../include/maddsua/http.hpp"

bool maddsua::socketsReady() {
	bool result = true;

    SOCKET temp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (temp == INVALID_SOCKET){
		if (GetLastError() == WSANOTINITIALISED) result = false;
    }
    closesocket(temp);

	return result;
}

void maddsua::toLowerCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		if (text->at(i) >= 'A' && text->at(i) <= 'Z') {
			text->at(i) += 0x20;
		}
	}
}
std::string maddsua::toLowerCase(std::string text) {
	std::string temp = text;
	toLowerCase(&temp);
	return temp;
}

void maddsua::toUpperCase(std::string* text) {
	for (size_t c = 0; c < text->size(); c++) {
		if (text->at(c) >= 'a' && text->at(c) <= 'z') text->at(c) -= 0x20;
	}
}
std::string maddsua::toUpperCase(std::string text) {
	std::string temp = text;
	toUpperCase(&temp);
	return temp;
}

void maddsua::toTitleCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		if (((i >= 1) ? text->at(i - 1) : '-') == '-' && (text->at(i) >= 'a' && text->at(i) <= 'z')) text->at(i) -= 0x20;
	}
}
std::string maddsua::toTitleCase(std::string text) {
	std::string temp = text;
	toTitleCase(&temp);
	return temp;
}

void maddsua::trim(std::string* text) {
	//	list of characters to remove
	const std::string wsc = "\r\n\t ";

	//	forward pass
	size_t pos_from = 0;
	while (pos_from < text->size()) {
		bool start_iswsc = false;
		for (auto cc : wsc) {
			if (text->at(pos_from) == cc) {
				start_iswsc = true;
				break;
			}
		}
		if (start_iswsc) pos_from++;
			else break;
	}

	//	backward pass
	size_t pos_to = text->size() - 1;
	while (pos_to >= 0) {
		bool end_iswsc = false;
		for (auto cc : wsc) {
			if (text->at(pos_to) == cc) {
				end_iswsc = true;
				break;
			}
		}
		if (end_iswsc) pos_to--;
			else break;
	}
	
	std::string temp = text->substr(pos_from, 1 + pos_to - pos_from);
	
	*text = temp;
}
std::string maddsua::trim(std::string text) {
	std::string temp = text;
	trim(&temp);
	return temp;
}

//	I thought that was a good idea to code this part in C, for speed.
//	But here we go again, it probably contains a ton of explosive bugs.
//	Tested several times, looks ok, but still, be aware of
std::vector <std::string> maddsua::splitBy(std::string source, std::string token) {
	std::vector <std::string> temp;

	//	abort if source is empty
	if (!source.size()) return {};
	//	return entrire source is deliminator/token is empty
	if (!token.size()) return { source };

	//	return entrire source is deliminator/token is not present in the source
	auto match = source.find(token);
		if (match == std::string::npos) return { source };

	//	iterate trough the res of the string
	while (match != std::string::npos) {
		if (match > 0) temp.push_back(source.substr(0, match));
		source = source.substr(match + token.size());
		match = source.find(token);
	}
	
	//	push the remaining part
	if (source.size()) temp.push_back(source);
	//	done
	return temp;
}

std::string maddsua::findHeader(std::string headerName, std::vector <maddsua::datapair>* headers) {
	for (auto headerObject : *headers) {
		if (toLowerCase(headerObject.name) == toLowerCase(headerName)) {
			return headerObject.value;
		}
	}
	return "";
}

maddsua::httpRequest maddsua::_getData(SOCKET* client) {

	//	receive http header first
	std::string dataStageOne;
	auto headerEnded = std::string::npos;
	while (headerEnded == std::string::npos) {

		char* chunk = (char*)malloc(HTTPLAMBDA_CHUNK_SIZE);
		auto bytesReceived = recv(*client, chunk, HTTPLAMBDA_CHUNK_SIZE, 0);
		if (bytesReceived <= 0) {
			//	drop the connection
			if (bytesReceived < 0) {
				free(chunk);
				break;
			}
			break;
		}
		dataStageOne.append(chunk, bytesReceived);
		//	"\r\n\r\n" - is a marker of header end
		headerEnded = dataStageOne.find("\r\n\r\n");
		free(chunk);
	}

	std::string requestHeaderText = dataStageOne.substr(0, headerEnded);

	//	split text by lines
	auto headerLines = splitBy(requestHeaderText.c_str(), "\r\n");
	/*std::vector <std::string> startArgs;
	{
		size_t startpoint = 0;
		
		for (size_t i = 0; i < requestHeaderText.size(); i++) {
			
			if ((((i >= 1) ? requestHeaderText[i - 1] : 0x00) == '\r' && requestHeaderText[i] == '\n')) {

				headerLines.push_back(requestHeaderText.substr(startpoint, i - startpoint - 1));
				startpoint = i + 1;
			}
		}

		headerLines.push_back(requestHeaderText.substr(startpoint));
	}*/

	//	parse start-line
	std::vector <std::string> startArgs;
	if (headerLines.size() >= 1) startArgs = splitBy(headerLines[0].c_str(), " ");
	/*if (headerLines.size() >= 1) {

		auto startLine = headerLines[0];
		size_t startpoint = 0;

		for (size_t i = 0; i < startLine.size(); i++) {

			if (startLine[i] == ' ') {
				startArgs.push_back(startLine.substr(startpoint, i - startpoint));
				startpoint = i + 1;
			}
		}

		startArgs.push_back(startLine.substr(startpoint));
	}*/

	//	parse headers
	std::vector <maddsua::datapair> headers;
	{
		for (size_t i = 1; i < headerLines.size(); i++) {
			auto hLine = headerLines[i];
			auto delim = hLine.find_first_of(':');

			if (delim == std::string::npos) break;

			headers.push_back({
				toLowerCase(trim(hLine.substr(0, delim))),
				trim(hLine.substr(delim + 1))
			});

			/*auto pair = splitBy(headerLines[i].c_str(), ":");
			if (pair.size() >= 2) headers.push_back({
				toLowerCase(trim(pair[0])),
				trim(pair[1])
			});*/
		}
	}

	//	download body is exists
	std::string requestBody;
	{
		auto contentLength = findHeader("Content-Length", &headers);
		if (contentLength.size()) {
			size_t bodySize;

			try { bodySize = std::stoi(contentLength); }
			catch(...) { bodySize = 0; }

			if (bodySize) {
				requestBody = dataStageOne.substr(headerEnded + 4);
				const size_t remains = (requestBody.size() < bodySize) ? (bodySize - requestBody.size()) : 0;

				if (remains) {
					std::string temp;
					while (temp.size() < remains) {

						char* chunk = (char*)malloc(HTTPLAMBDA_CHUNK_SIZE);
						auto bytesReceived = recv(*client, chunk, HTTPLAMBDA_CHUNK_SIZE, 0);
						if (bytesReceived <= 0) {
							//	drop the connection
							if (bytesReceived < 0) {
								free(chunk);
								puts("Conntection interrupted while receiving data");
								break;
								//return;
							}
							break;
						}
						temp.append(chunk, bytesReceived);
						free(chunk);
					}
				}
			}
		}
	}

    return {
		startArgs,
		headers,
		requestBody
	};
}

maddsua::actionResult maddsua::_sendData(SOCKET* client, std::string startline, std::vector <datapair>* headers, std::string* body) {

	//	create reaponse message
	auto temp = startline + "\r\n";

	//	add headers
	for (auto header : *headers) {
		temp += header.name + ": " + header.value + "\r\n";
	}

	//	add content length header
	if (body->size() && !findHeader("Content-Length", headers).size())
		temp += "Content-Length: " + std::to_string(body->size()) + "\r\n";
	
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

std::string maddsua::formattedTime(time_t epoch_time) {
	char timebuff[128];
	tm tms = *gmtime(&epoch_time);
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &tms);
	return timebuff;
}

std::vector <maddsua::datapair> maddsua::searchQueryParams(std::string* url) {
	std::vector <maddsua::datapair> params;

	auto startpoint = url->find_last_of('?');
		if (startpoint == std::string::npos || startpoint >= (url->size() - 1)) return params;

	auto pairs = splitBy(url->substr(startpoint + 1).c_str(), "&");
	for (auto param : pairs) {
		auto pair = splitBy(param.c_str(), "=");
		if (pair.size() >= 2) params.push_back({ pair[0], pair[1] });
	}

	return params;
}
