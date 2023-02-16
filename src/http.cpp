/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include "../include/maddsua/http.hpp"


struct _httpcode {
	unsigned int code;
	const char* textCode;
};

//	kindly borrowed from https://github.com/anonkey/http-status-code-json/blob/master/index.json
const std::vector <_httpcode> httpCodeList = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 102, "Processing" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 207, "Multi-Status" },
	{ 226, "IM Used" },
	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 308, "Permanent Redirect" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Payload Too Large" },
	{ 414, "URI Too Long" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Range Not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 418, "I'm a teapot" },
	{ 422, "Unprocessable Entity" },
	{ 423, "Locked" },
	{ 424, "Failed Dependency" },
	{ 426, "Upgrade Required" },
	{ 428, "Precondition Required" },
	{ 429, "Too Many Requests" },
	{ 431, "Request Header Fields Too Large" },
	{ 451, "Unavailable For Legal Reasons" },
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Time-out" },
	{ 505, "HTTP Version Not Supported" },
	{ 506, "Variant Also Negotiates" },
	{ 507, "Insufficient Storage" },
	{ 511, "Network Authentication Required" }
};

std::string lambda::httpStatusString(const unsigned int statusCode) {
	for (auto status : httpCodeList) {
		if (status.code == statusCode)
			return (std::to_string(status.code) + " " + status.textCode);
	}
	return "200 OK";
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

void lambda::toLowerCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		if (text->at(i) >= 'A' && text->at(i) <= 'Z') text->at(i) += 0x20;
	}
}
std::string lambda::toLowerCase(std::string text) {
	std::string temp = text;
	toLowerCase(&temp);
	return temp;
}

void lambda::toUpperCase(std::string* text) {
	for (size_t c = 0; c < text->size(); c++) {
		if (text->at(c) >= 'a' && text->at(c) <= 'z') text->at(c) -= 0x20;
	}
}
std::string lambda::toUpperCase(std::string text) {
	std::string temp = text;
	toUpperCase(&temp);
	return temp;
}

void lambda::toTitleCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		auto prev = ((i >= 1) ? text->at(i - 1) : '-');
		if ((prev == '-' || prev == ' ') && (text->at(i) >= 'a' && text->at(i) <= 'z')) text->at(i) -= 0x20;
	}
}
std::string lambda::toTitleCase(std::string text) {
	std::string temp = text;
	toTitleCase(&temp);
	return temp;
}

void lambda::trim(std::string* text) {
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
std::string lambda::trim(std::string text) {
	std::string temp = text;
	trim(&temp);
	return temp;
}

//	I thought that was a good idea to code this part in C, for speed.
//	But here we go again, it probably contains a ton of explosive bugs.
//	Tested several times, looks ok, but still, be aware of
std::vector <std::string> lambda::splitBy(std::string source, std::string token) {
	std::vector <std::string> temp;

	//	abort if source is empty
	if (!source.size()) return {};
	//	return entrire source is deliminator/token is empty
	if (!token.size()) return { source };

	//	return entrire source is deliminator/token is not present in the source
	auto match = source.find(token);
		if (match == std::string::npos) return { source };

	//	iterate trough the res of the string
	size_t startpos = 0;
	while (match != std::string::npos) {
		temp.push_back(source.substr(startpos, match - startpos));
		startpos = match + token.size();
		match = source.find(token, startpos);
	}
	
	//	push the remaining part
	if (source.size() - startpos) temp.push_back(source.substr(startpos));
	//	done
	return temp;
}

std::string lambda::findHeader(std::string headerName, std::vector <lambda::datapair>* headers) {
	for (auto headerObject : *headers) {
		if (toLowerCase(headerObject.name) == toLowerCase(headerName))
			return headerObject.value;
	}
	return {};
}

std::string lambda::httpTime(time_t epoch_time) {
	char timebuff[128];
	tm tms = *gmtime(&epoch_time);
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &tms);
	return timebuff;
}

std::vector <lambda::datapair> lambda::getSearchQuery(std::string* url) {
	std::vector <lambda::datapair> params;

	auto startpoint = url->find_last_of('?');
		if (startpoint == std::string::npos || startpoint >= (url->size() - 1)) return params;

	auto pairs = splitBy(url->substr(startpoint + 1).c_str(), "&");
	for (auto param : pairs) {
		auto pair = splitBy(param.c_str(), "=");
		if (pair.size() >= 2) params.push_back({ pair[0], pair[1] });
	}

	return params;
}

void lambda::insertHeader(std::string header, std::string value, std::vector <lambda::datapair>* headers) {

	for (auto& h : *headers) {
		if (toLowerCase(h.name) == toLowerCase(header)) {
			toTitleCase(&h.name);
			h.value = value;
			return;
		}
	}
	
	headers->push_back({toTitleCase(header), value});
}