#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

KVContainer::KVContainer(const KVContainer& other) {
	this->m_data = other.m_data;
}

KVContainer::KVContainer(KVContainer&& other) {
	this->m_data = std::move(other.m_data);
}

KVContainer::KVContainer(const std::initializer_list<KVpair>& init) {
	for (const auto& entry : init) {

		const auto keyNormalized = Strings::toLowerCase(entry.first);
		const auto element = this->m_data.find(keyNormalized);

		if (element == this->m_data.end()) {
			this->m_data[keyNormalized] = { entry.second };
			continue;
		}

		element->second.push_back(entry.second);
	}
}

KVContainer& KVContainer::operator=(const KVContainer& other) noexcept {
	this->m_data = other.m_data;
	return *this;
}

KVContainer& KVContainer::operator=(KVContainer&& other) noexcept {
	this->m_data = std::move(other.m_data);
	return *this;
}

bool KVContainer::has(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	return this->m_data.contains(keyNormalized);
}

void KVContainer::del(const std::string& key) {
	const auto keyNormalized = Strings::toLowerCase(key);
	if (!this->m_data.contains(keyNormalized)) return;
	this->m_data.erase(keyNormalized);
}

void KVContainer::set(const std::string& key, const std::string value) {
	const auto keyNormalized = Strings::toLowerCase(key);
	this->m_data[keyNormalized] = { value };
}

void KVContainer::append(const std::string& key, const std::string value) {

	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->m_data.find(keyNormalized);

	if (element == this->m_data.end()) {
		this->m_data[keyNormalized] = { value };
		return;
	}

	element->second.push_back(value);
}

std::string KVContainer::get(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->m_data.find(keyNormalized);
	if (element == this->m_data.end()) return {};
	return element->second.size() ? element->second.at(0) : "";
}

std::vector<std::string> KVContainer::getAll(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->m_data.find(keyNormalized);
	if (element == this->m_data.end()) return {};
	return element->second;
}

std::vector<KVpair> KVContainer::entries() const {

	std::vector<KVpair> temp;

	for (const auto& entry : this->m_data) {
		for (const auto& value : entry.second) {
			temp.push_back({ entry.first, value });
		}
	}

	return temp;
}

size_t KVContainer::size() const noexcept {
	return this->m_data.size();
}
