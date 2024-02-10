#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

#include <iterator>

using namespace Lambda::HTTP;

Cookies::Cookies(const std::string& cookiestring) {

	auto entries = Strings::split(cookiestring, "; ");

	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = Strings::trim(entry.substr(0, kvSeparatorIdx));
		auto value = Strings::trim(entry.substr(kvSeparatorIdx + 1));

		if (!key.size() || !value.size()) continue;

		this->m_data[key] = { value };
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
	return *this;
}

Cookies& Cookies::operator=(Cookies&& other) noexcept {
	this->m_data = std::move(other.m_data);
	return *this;
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

void Cookies::set(const std::string& key, const std::string& value, const std::initializer_list<CookieParams>& props) {
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
		auto cookie = entry.first + '=' + entry.second.value;
		for (const auto& prop : entry.second.props) {
			if (!prop.key.size()) continue;
			cookie += "; " + (prop.value.size() ? (prop.key + '=' + prop.value) : prop.key);
		}
		temp.push_back(cookie);
	}
	return temp;
}

Cookies::CookieParams::CookieParams(const std::string& init) {
	this->key = init;
}

Cookies::CookieParams::CookieParams(const char* init) {
	this->key = init;
}

Cookies::CookieParams::CookieParams(std::initializer_list<std::string> init) {

	for (auto itr = init.begin(); itr != init.end(); itr++) {

		const auto idx = std::distance(init.begin(), itr);
		const auto& item = *itr;

		switch (idx) {

			case 0: {
				this->key = item;
			} break;

			case 1: {
				this->value = item;
			} break;
		
			default: return;
		}
	}
}
