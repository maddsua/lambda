#include "./http.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda::HTTP;

void URLSearchParams::fromHref(const std::string& URLString) {

	auto qmark = URLString.find_last_of('?');
	auto queries = stringSplit(qmark != std::string::npos ? URLString.substr(qmark + 1) : URLString, "&");
	
	for (const auto& item : queries) {

		if (item.find('=') == std::string::npos) continue;

		auto query = stringSplit(item, "=");
		if (query.size() < 2) continue;

		const auto key = query.at(0);
		const auto value = query.at(1);

		data[stringToLowerCase(stringTrim(key))] = stringTrim(value);
	}
}

bool URLSearchParams::has(const std::string key) const {
	auto keyNormalized = stringToLowerCase(key);
	return this->data.find(keyNormalized) != this->data.end();
}

void URLSearchParams::set(const std::string key, const std::string& value) {
	auto keyNormalized = stringToLowerCase(key);
	this->data[keyNormalized] = Encoding::encodeURIComponent(value);
}

bool URLSearchParams::append(const std::string key, const std::string& value) {
	auto keyNormalized = stringToLowerCase(key);
	if (has(keyNormalized)) return false;
	set(keyNormalized, value);
	return true;
}

std::string URLSearchParams::get(const std::string key) const {
	auto keyNormalized = stringToLowerCase(key);
	if (!has(keyNormalized)) return {};
	return this->data.at(keyNormalized);
}

void URLSearchParams::del(const std::string key) {
	auto keyNormalized = stringToLowerCase(key);
	if (!has(keyNormalized)) return;
	this->data.erase(keyNormalized);
}

std::string URLSearchParams::stringify() const {
	auto result = std::string();
	for (auto item : this->data) {
		result += (result.size() ? "&" : "") + item.first + "=" + item.second;
	}
	return result;
}

std::vector<KVtype> URLSearchParams::entries() const {
	auto entries = std::vector<KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}

size_t URLSearchParams::length() const {
	return this->data.size();
}
