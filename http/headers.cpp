#include "./http.hpp"

HTTP::Headers::Headers() {
	//	do nothing lol
}

HTTP::Headers::Headers(const std::string& httpHeader) {

	auto headerLines = stringSplit(httpHeader, "\r\n");

	for (auto& item : headerLines) {

		auto kvSeparatorIdx = item.find(':');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == item.size() - 1) continue;

		auto key = item.substr(0, kvSeparatorIdx);
		auto value = item.substr(kvSeparatorIdx + 1);

		HTTP::stringTrim(key);
		HTTP::stringTrim(value);
		HTTP::stringToTittleCase(key);

		this->data[key] = value;
	}
}

void HTTP::Headers::fromEntries(const std::vector<HTTP::KVtype>& headers) {
	this->data.clear();
	for (auto& item : headers) {
		if (!item.key.size() || !item.value.size()) continue;
		auto key = stringToTittleCase(item.key);
		this->data[key] = item.value;
	}
}

bool HTTP::Headers::has(std::string key) {
	stringToTittleCase(key);
	return this->data.find(key) != this->data.end();
}

void HTTP::Headers::set(std::string key, const std::string& value) {
	stringToTittleCase(key);
	this->data[key] = value;
}

bool HTTP::Headers::append(std::string key, const std::string& value) {
	if (has(key)) return false;
	set(key, value);
	return true;
}

std::string HTTP::Headers::get(std::string key) {
	stringToTittleCase(key);
	return has(key) ? this->data[key] : std::string();
}

void HTTP::Headers::del(std::string key) {
	stringToTittleCase(key);
	if (!has(key)) return;
	this->data.erase(key);
}

std::string HTTP::Headers::stringify() {
	auto result = std::string();
	for (auto item : this->data) {
		result += item.first + ": " + item.second + "\r\n";
	}
	return result;
}

std::vector<HTTP::KVtype> HTTP::Headers::entries() {
	auto entries = std::vector<HTTP::KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}
