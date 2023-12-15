#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

KVContainer::KVContainer(const std::vector<KVpair>& entries) {

	for (const auto& entry : entries) {

		const auto keyNormalized = Strings::toLowerCase(entry.key);
		const auto element = this->data.find(keyNormalized);

		if (element == this->data.end()) {
			this->data[keyNormalized] = { entry.value };
			continue;
		}

		element->second.push_back(entry.value);
	}
}

bool KVContainer::has(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	return this->data.contains(keyNormalized);
}

void KVContainer::del(const std::string& key) {
	const auto keyNormalized = Strings::toLowerCase(key);
	if (!this->data.contains(keyNormalized)) return;
	this->data.erase(keyNormalized);
}

void KVContainer::set(const std::string& key, const std::string value) {
	const auto keyNormalized = Strings::toLowerCase(key);
	this->data[keyNormalized] = { value };
}

void KVContainer::append(const std::string& key, const std::string value) {

	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->data.find(keyNormalized);

	if (element == this->data.end()) {
		this->data[keyNormalized] = { value };
		return;
	}

	element->second.push_back(value);
}

std::string KVContainer::get(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->data.find(keyNormalized);
	if (element == this->data.end()) return {};
	return element->second.size() ? element->second.at(0) : "";
}

std::vector<std::string> KVContainer::getAll(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->data.find(keyNormalized);
	if (element == this->data.end()) return {};
	return element->second;
}

std::vector<KVpair> KVContainer::entries() const {

	std::vector<KVpair> temp;

	for (const auto& entry : this->data) {
		for (const auto& value : entry.second) {
			temp.push_back({ entry.first, value });
		}
	}

	return temp;
}
