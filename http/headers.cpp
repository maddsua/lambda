#include "./http.hpp"

using namespace Lambda::HTTP;

void Headers::fromHTTP(const std::string& httpHeaders) {

	auto headerLines = stringSplit(httpHeaders, "\r\n");

	for (auto& item : headerLines) {

		auto kvSeparatorIdx = item.find(':');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == item.size() - 1) continue;

		auto key = item.substr(0, kvSeparatorIdx);
		auto value = item.substr(kvSeparatorIdx + 1);

		stringTrim(key);
		stringTrim(value);
		stringToTittleCase(key);

		this->data[key] = value;
	}
}

void Headers::fromEntries(const std::vector<KVtype>& headers) {
	this->data.clear();
	for (auto& item : headers) {
		if (!item.key.size() || !item.value.size()) continue;
		auto key = stringToTittleCase(item.key);
		this->data[key] = item.value;
	}
}

bool Headers::has(std::string key) {
	stringToTittleCase(key);
	return this->data.find(key) != this->data.end();
}

void Headers::set(std::string key, const std::string& value) {
	stringToTittleCase(key);
	this->data[key] = value;
}

bool Headers::append(std::string key, const std::string& value) {
	if (has(key)) return false;
	set(key, value);
	return true;
}

std::string Headers::get(std::string key) {
	stringToTittleCase(key);
	return has(key) ? this->data[key] : std::string();
}

void Headers::del(std::string key) {
	stringToTittleCase(key);
	if (!has(key)) return;
	this->data.erase(key);
}

std::string Headers::stringify() {
	auto result = std::string();
	for (auto item : this->data) {
		result += item.first + ": " + item.second + "\r\n";
	}
	return result;
}

std::vector<KVtype> Headers::entries() {
	auto entries = std::vector<KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}
