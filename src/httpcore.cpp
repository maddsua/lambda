/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include "../include/lambda/httpcore.hpp"

#include <time.h>

/*
	Misc text operations
*/

void lambda::toLowerCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		if (text->at(i) >= 'A' && text->at(i) <= 'Z') text->at(i) += 0x20;
	}
}
std::string lambda::toLowerCase(std::string text) {
	toLowerCase(&text);
	return text;
}

void lambda::toUpperCase(std::string* text) {
	for (size_t c = 0; c < text->size(); c++) {
		if (text->at(c) >= 'a' && text->at(c) <= 'z') text->at(c) -= 0x20;
	}
}
std::string lambda::toUpperCase(std::string text) {
	toUpperCase(&text);
	return text;
}

void lambda::toTitleCase(std::string* text) {
	for (size_t i = 0; i < text->size(); i++) {
		auto prev = ((i >= 1) ? text->at(i - 1) : '-');
		if ((prev == '-' || prev == ' ') && (text->at(i) >= 'a' && text->at(i) <= 'z')) text->at(i) -= 0x20;
	}
}
std::string lambda::toTitleCase(std::string text) {
	toTitleCase(&text);
	return text;
}

void lambda::trimString(std::string* text) {

	//	list of characters to remove
	static const std::string whitespaceChars = "\r\n\t ";

	//	forward pass
	size_t pos_from = 0;
	while (pos_from < text->size()) {
		bool start_iswsc = false;
		for (auto wsc : whitespaceChars) {
			if (text->at(pos_from) == wsc) {
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
		for (auto wsc : whitespaceChars) {
			if (text->at(pos_to) == wsc) {
				end_iswsc = true;
				break;
			}
		}
		if (end_iswsc) pos_to--;
			else break;
	}
		
	*text = text->substr(pos_from, 1 + pos_to - pos_from);
}

std::string lambda::trimString(std::string text) {
	trimString(&text);
	return text;
}

std::vector <std::string> lambda::splitBy(const std::string& source, const std::string token) {

	std::vector <std::string> result;

	//	abort if source is empty
	if (!source.size()) return {};
	//	return entrire source if deliminator/token is empty
	if (!token.size()) return { source };

	//	return entrire source is deliminator/token is not present in the source
	auto match = source.find(token);
	if (match == std::string::npos) return { source };

	//	iterate trough the res of the string
	size_t startpos = 0;
	while (match != std::string::npos) {
		result.push_back(source.substr(startpos, match - startpos));
		startpos = match + token.size();
		match = source.find(token, startpos);
	}
	
	//	push the remaining part
	if (source.size() - startpos) result.push_back(source.substr(startpos));

	//	done
	return result;
}


/*
	HTTP header class
*/

lambda::httpHeaders::httpHeaders(const std::vector <lambda::stringPair>& headers) {
	content = headers;
}

size_t lambda::httpHeaders::parse(const std::string& text) {
	
	auto headerLines = splitBy(text, "\r\n");
	if (!headerLines.size()) return 0;
	return parse(headerLines);
}

size_t lambda::httpHeaders::parse(const std::vector <std::string>& lines) {

	if (content.size()) content.clear();

	for (auto& item : lines) {
		auto delim = item.find_first_of(':');
		if (delim == std::string::npos) continue;
		content.push_back({ toTitleCase(trimString(item.substr(0, delim))), trimString(item.substr(delim + 1)) });
	}

	return content.size();
}

bool lambda::httpHeaders::exists(std::string header) {
	
	toTitleCase(&header);

	for (auto& item : content) {
		if (item.key == header) return true;
	}

	return false;
}

void lambda::httpHeaders::set(std::string header, std::string value) {

	toTitleCase(&header);
	trimString(&header);

	for (auto& item : content) {
		if (item.key == header) {
			item.value = value;
			return;
		}
	}

	content.push_back({ header, value });
}

bool lambda::httpHeaders::add(std::string header, std::string value) {
	if (exists(header)) return false;
	set(header, value);
	return true;
}

std::string lambda::httpHeaders::find(std::string header) {

	toTitleCase(&header);

	for (auto& item : content) {
		if (item.key == header) return item.value;
	}

	return {};
}

bool lambda::httpHeaders::remove(std::string header) {

	toTitleCase(&header);

	for (auto itr = content.begin(); itr != content.end(); itr++) {
		if ((*itr).key == header) {
			content.erase(itr);
			return true;
		}
	}

	return false;
}

std::vector <lambda::stringPair> lambda::httpHeaders::list() {
	return content;
}

std::string lambda::httpHeaders::dump() {

	std::string result;

	for (auto item : content)
		result += item.key + ": " + item.value + "\r\n";

	return result;
}


/*
	Search query class
*/

lambda::httpSearchQuery::httpSearchQuery(const std::string& url) { parse(url); }

size_t lambda::httpSearchQuery::parse(const std::string& url) {

	if (!content.size()) content.clear();

	auto qmark = url.find_last_of('?');
	if (qmark == std::string::npos || qmark > (url.size() - 4)) return 0;

	auto queries = splitBy(url.substr(qmark + 1), "&");
	for (auto& item : queries) {
		auto pair = splitBy(item, "=");
		if (pair.size() != 2) continue;
		content.push_back({ toLowerCase(pair[0]), pair[1] });
	}

	return content.size();
}

bool lambda::httpSearchQuery::exists(std::string query) {

	toLowerCase(&query);

	for (auto& item : content) {
		if (item.key == query) return true;
	}

	return false;
}

void lambda::httpSearchQuery::set(stringPair query) {

	toLowerCase(&query.key);

	for (auto& item : content) {
		if (toLowerCase(item.key) == toLowerCase(query.key)) {
			item.value = query.value;
			return;
		}
	}

	content.push_back(query);
}

bool lambda::httpSearchQuery::add(stringPair query) {
	if (exists(query.key)) return false;
	set(query);
	return true;
}

bool lambda::httpSearchQuery::remove(std::string query) {

	toLowerCase(&query);

	for (auto itr = content.begin(); itr != content.end(); itr++) {
		if ((*itr).key == query) {
			content.erase(itr);
			return true;
		}
	}

	return false;
}

std::string lambda::httpSearchQuery::find(std::string query) {

	toLowerCase(&query);

	for (auto& item : content) {
		if (item.key == query) return item.value;
	}

	return {};
}

std::string lambda::httpSearchQuery::dump() {

	std::string result;

	for (auto item : content)
		result += std::string(result.size() ? "&" : "?") + item.key + "=" + item.value;

	return result;
}


/*
	Extended JS-like string class
*/

lambda::jstring::jstring(const std::string& ccppstring) {
	sstring = ccppstring;
}

bool lambda::jstring::includes(const std::string& substring) {
	return sstring.find(substring) != std::string::npos;
}

bool lambda::jstring::includes(const std::vector <std::string>& substrings) {

	for (auto item : substrings) {
		if (sstring.find(item) != std::string::npos) return true;
	}

	return false;
}

bool lambda::jstring::endsWith(const std::string& substring) {
	return sstring.find(substring) == (sstring.size() - substring.size());
}

bool lambda::jstring::startsWith(const std::string& substring) {
	return sstring.find(substring) == 0;
}

void lambda::jstring::toLowerCase() {
	lambda::toLowerCase(&sstring);
}

void lambda::jstring::toTitleCase() {
	lambda::toTitleCase(&sstring);
}

void lambda::jstring::toUpperCase() {
	lambda::toUpperCase(&sstring);
}


/*
	Time functions
*/

std::string lambda::httpTime(time_t epoch_time) {
	char timebuff[128];
	tm tms = *gmtime(&epoch_time);
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &tms);
	return timebuff;
}

std::string lambda::httpTime() {
	return lambda::httpTime(time(nullptr));
}


