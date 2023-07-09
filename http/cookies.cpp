#include "./http.hpp"

using namespace Lambda::HTTP;

void Cookies::fromRequest(const Request& request) {
	auto headers = request.headers;
	auto headerCookie = headers.get("Cookie");
	if (!headerCookie.size()) return;
	fromString(headerCookie);
}

void Cookies::fromString(const std::string& cookies) {

	auto entries = stringSplit(cookies, "; ");
	this->data.clear();

	for (auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = entry.substr(0, kvSeparatorIdx);
		auto value = entry.substr(kvSeparatorIdx + 1);

		stringTrim(key);
		stringTrim(value);

		this->data[key] = value;
	}
}

bool Cookies::has(const std::string key) {
	auto keyNormalized = stringToLowerCase(key);
	return this->data.find(keyNormalized) != this->data.end();
}

void Cookies::set(const std::string key, const std::string& value) {
	auto keyNormalized = stringToLowerCase(key);
	this->data[keyNormalized] = value;
}

std::string Cookies::get(const std::string key) {
	auto keyNormalized = stringToLowerCase(key);
	if (!has(keyNormalized)) return {};
	return this->data[keyNormalized];
}

void Cookies::del(const std::string key) {
	auto keyNormalized = stringToLowerCase(key);
	if (!has(keyNormalized)) return;
	this->data.erase(keyNormalized);
}

std::string Cookies::stringify() {
	auto result = std::string();
	for (auto item : this->data) {
		if (result.size()) result += "; ";
		result += item.first + "=" + item.second;
	}
	return result;
}

std::vector<KVtype> Cookies::entries() {
	auto entries = std::vector<KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}

KVtype Cookies::toHeader() {
	return { "Set-Cookie", stringify() };
}
