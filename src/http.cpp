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
		auto prev = ((i >= 1) ? text->at(i - 1) : '-');
		if ((prev == '-' || prev == ' ') && (text->at(i) >= 'a' && text->at(i) <= 'z')) text->at(i) -= 0x20;
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

std::string maddsua::headerFind(std::string headerName, std::vector <maddsua::datapair>* headers) {
	for (auto headerObject : *headers) {
		if (toLowerCase(headerObject.name) == toLowerCase(headerName)) {
			return headerObject.value;
		}
	}
	return "";
}

std::string maddsua::httpTime(time_t epoch_time) {
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

void maddsua::headerInsert(std::string header, std::string value, std::vector <maddsua::datapair>* headers) {

	for (auto& h : *headers) {
		if (toLowerCase(h.name) == toLowerCase(header)) {
			toTitleCase(&h.name);
			h.value = value;
			return;
		}
	}
	
	headers->push_back({toTitleCase(header), value});
}