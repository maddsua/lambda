#include "./http.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda::HTTP;

void URLSearchParams::fromHref(const std::string& URLString) {

	auto qmark = URLString.find_last_of('?');
	auto queries = stringSplit(qmark != std::string::npos ? URLString.substr(qmark + 1) : URLString, "&");
	
	for (auto& item : queries) {

		if (item.find('=') == std::string::npos) continue;

		auto query = stringSplit(item, "=");
		if (query.size() < 2) continue;

		const auto key = query.at(0);
		const auto value = query.at(1);

		data[stringToLowerCase(stringTrim(key))] = stringTrim(value);
	}
}

bool URLSearchParams::has(std::string key) {
	stringToLowerCase(key);
	return this->data.find(key) != this->data.end();
}

void URLSearchParams::set(std::string key, const std::string& value) {
	stringToLowerCase(key);
	this->data[key] = Encoding::encodeURIComponent(value);
}

bool URLSearchParams::append(std::string key, const std::string& value) {
	if (has(key)) return false;
	set(key, value);
	return true;
}

std::string URLSearchParams::get(std::string key) {
	stringToLowerCase(key);
	return has(key) ? std::string() : this->data[key];
}

void URLSearchParams::del(std::string key) {
	stringToLowerCase(key);
	if (!has(key)) return;
	this->data.erase(key);
}

std::string URLSearchParams::stringify() {
	auto result = std::string();
	for (auto item : this->data) {
		result += (result.size() ? "&" : "") + item.first + "=" + item.second;
	}
	return result;
}

std::vector<KVtype> URLSearchParams::entries() {
	auto entries = std::vector<KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}

size_t URLSearchParams::length() {
	return this->data.size();
}
