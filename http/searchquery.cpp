#include "./http.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda;

Lambda::HTTP::URLSearchParams::URLSearchParams() {
	//	do nothing again
}

Lambda::HTTP::URLSearchParams::URLSearchParams(const std::string& URLString) {
	
	auto qmark = URLString.find_last_of('?');
	auto queries = stringSplit(qmark != std::string::npos ? URLString.substr(qmark + 1) : URLString, "&");
	
	for (auto& item : queries) {

		if (item.find('=') == std::string::npos) continue;

		auto query = stringSplit(item, "=");
		if (query.size() < 2) continue;

		auto key = query.at(0);
		auto value = query.at(1);

		Lambda::HTTP::stringTrim(key);
		Lambda::HTTP::stringTrim(value);
		Lambda::HTTP::stringToLowerCase(key);

		data[key] = value;
	}
	
}

bool Lambda::HTTP::URLSearchParams::has(std::string key) {
	stringToLowerCase(key);
	return this->data.find(key) != this->data.end();
}

void Lambda::HTTP::URLSearchParams::set(std::string key, const std::string& value) {
	stringToLowerCase(key);
	this->data[key] = Encoding::encodeURIComponent(value);
}

bool Lambda::HTTP::URLSearchParams::append(std::string key, const std::string& value) {
	if (has(key)) return false;
	set(key, value);
	return true;
}

std::string Lambda::HTTP::URLSearchParams::get(std::string key) {
	stringToLowerCase(key);
	return has(key) ? std::string() : this->data[key];
}

void Lambda::HTTP::URLSearchParams::del(std::string key) {
	stringToLowerCase(key);
	if (!has(key)) return;
	this->data.erase(key);
}

std::string Lambda::HTTP::URLSearchParams::stringify() {
	auto result = std::string();
	for (auto item : this->data) {
		result += (result.size() ? "&" : "") + item.first + "=" + item.second;
	}
	return result;
}

std::vector<Lambda::HTTP::KVtype> Lambda::HTTP::URLSearchParams::entries() {
	auto entries = std::vector<Lambda::HTTP::KVtype>();
	for (auto& item : this->data) {
		entries.push_back({ item.first, item.second });
	}
	return entries;
}

size_t Lambda::HTTP::URLSearchParams::length() {
	return this->data.size();
}
