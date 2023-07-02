#include "./http.hpp"

using namespace Lambda::HTTP;

void Headers::fromHTTP(const std::string& httpHeaders) {

	auto headerLines = stringSplit(httpHeaders, "\r\n");

	for (const auto& item : headerLines) {

		auto kvSeparatorIdx = item.find(':');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == item.size() - 1) continue;

		auto key = item.substr(0, kvSeparatorIdx);
		auto value = item.substr(kvSeparatorIdx + 1);

		stringTrim(key);
		stringTrim(value);
		stringToTittleCase(key);

		this->data.push_back({ key, value });
	}
}

void Headers::fromEntries(const std::vector<KVtype>& headers) {
	this->data.clear();
	for (const auto& item : headers) {
		if (!item.key.size() || !item.value.size()) continue;
		auto key = stringToTittleCase(item.key);
		this->data.push_back({ key, item.value });
	}
}

bool Headers::has(const std::string& key) {
	auto keyNormalized = stringToTittleCase(key);
	for (const auto& item : this->data) {
		if (item.key != keyNormalized) continue;
		return true;
	}
	return false;
}

std::string Headers::get(const std::string& key) {
	auto keyNormalized = stringToTittleCase(key);
	for (const auto& item : this->data) {
		if (item.key != keyNormalized) continue;
		return item.value;
	}
	return {};
}

std::vector<std::string> Headers::getMultiValue(const std::string& key) {
	auto keyNormalized = stringToTittleCase(key);
	std::vector<std::string> result;
	for (const auto& item : this->data) {
		if (item.key != keyNormalized) continue;
		result.push_back(item.value);
	}
	return result;
}

void Headers::set(const std::string& key, const std::string& value) {
	auto keyNormalized = stringToTittleCase(key);
	for (auto& item : this->data) {
		if (item.key != keyNormalized) continue;
		item.value = value;
		return;
	}
	this->data.push_back({ keyNormalized, value });
}

bool Headers::append(const std::string& key, const std::string& value) {
	stringToTittleCase(key);
	this->data.push_back({ key, value });
	return true;
}

void Headers::del(const std::string& key) {
	auto keyNormalized = stringToTittleCase(key);
	for (size_t idx = 0; idx < this->data.size();) {
		if (this->data.at(idx).key == keyNormalized) {
			this->data.erase(this->data.begin() + idx);
			continue;
		}
		idx++;
	}
}

std::string Headers::stringify() {
	auto result = std::string();
	for (const auto& item : this->data) {
		result += item.key + ": " + item.value + "\r\n";
	}
	return result;
}

const std::vector<KVtype>& Headers::entries() {
	return this->data;
}
