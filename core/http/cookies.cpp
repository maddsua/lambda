#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

Cookies::Cookies(const std::string& init) {

	auto entries = Strings::split(init, "; ");

	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = Strings::trim(entry.substr(0, kvSeparatorIdx));
		auto value = Strings::trim(entry.substr(kvSeparatorIdx + 1));

		if (!key.size() || !value.size()) continue;

		this->m_data[key] = { value };
	}
}

Cookies::Cookies(const std::initializer_list<KVpair>& init) {
	for (const auto& entry : init) {
		const auto keyNormalized = Strings::toLowerCase(entry.first);
		const auto element = this->m_data.find(keyNormalized);
		this->m_data[keyNormalized] = entry.second;
		this->m_set_queue[keyNormalized] = { entry.second };
	}
}

Cookies::Cookies(const Cookies& other) {
	this->m_data = other.m_data;
}

Cookies::Cookies(Cookies&& other) {
	this->m_data = std::move(other.m_data);
}

Cookies& Cookies::operator=(const Cookies& other) noexcept {
	this->m_data = other.m_data;
}

Cookies& Cookies::operator=(Cookies&& other) noexcept {
	this->m_data = std::move(other.m_data);
}

std::string Cookies::get(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	const auto element = this->m_data.find(keyNormalized);
	if (element == this->m_data.end()) return {};
	return element->second;
}

bool Cookies::has(const std::string& key) const {
	const auto keyNormalized = Strings::toLowerCase(key);
	return this->m_data.contains(keyNormalized);
}

void Cookies::set(const std::string& key, const std::string& value) {
	const auto keyNormalized = Strings::toLowerCase(key);
	this->m_data[keyNormalized] = value;
	this->m_set_queue[keyNormalized] = { value };
}

void Cookies::set(const std::string& key, const std::string& value, const std::vector<std::string>& props) {
	const auto keyNormalized = Strings::toLowerCase(key);
	this->m_data[keyNormalized] = value;
	this->m_set_queue[keyNormalized] = { value, props };
}

void Cookies::del(const std::string& key) {
	const auto keyNormalized = Strings::toLowerCase(key);
	if (!this->m_data.contains(keyNormalized)) return;
	this->m_data.erase(keyNormalized);
	this->m_set_queue[keyNormalized] = { "", { "expires", "Thu, Jan 01 1970 00:00:00 UTC" } };
}

std::vector<KVpair> Cookies::entries() const {
	std::vector<KVpair> temp;
	for (const auto& entry : this->m_data)
		temp.push_back({ entry.first, entry.second });
	return temp;
}

size_t Cookies::size() const noexcept {
	return this->m_data.size();
}

std::vector<std::string> Cookies::serialize() const {
	std::vector<std::string> temp;
	for (const auto& entry : this->m_set_queue) {
		auto cookie = entry.first + '=' = entry.second.value;
		for (const auto prop : entry.second.props)
			cookie += "; " + prop;
		temp.push_back(cookie);
	}
	return temp;
}
