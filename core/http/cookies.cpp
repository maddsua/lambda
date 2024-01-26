#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

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
	this->setCookieQueue[keyNormalized] = { value };
}

void Cookies::set(const std::string& key, const std::string& value, const std::vector<CookieProp>& props) {
	const auto keyNormalized = Strings::toLowerCase(key);
	this->m_data[keyNormalized] = value;
	this->setCookieQueue[keyNormalized] = { value, props };
}

void Cookies::del(const std::string& key) {
	const auto keyNormalized = Strings::toLowerCase(key);
	if (!this->m_data.contains(keyNormalized)) return;
	this->m_data.erase(keyNormalized);
	this->setCookieQueue[keyNormalized] = { "", { "expires", "Thu, Jan 01 1970 00:00:00 UTC" } };
}

std::vector<KVpair> Cookies::entries() const {

	std::vector<KVpair> temp;

	for (const auto& entry : this->m_data) {
		temp.push_back({ entry.first, entry.second });
	}

	return temp;
}

size_t Cookies::size() const noexcept {
	return this->m_data.size();
}

std::vector<std::string> Cookies::serialize() const {
	
	std::vector<std::string> temp;

	for (const auto& entry : this->setCookieQueue) {

		auto cookie = entry.first + '=' = entry.second.value;

		for (const auto prop : entry.second.props) {
			if (prop.second.size()) {
				cookie += "; " + prop.first + '=' + prop.second;
			} else {
				cookie += "; " + prop.first;
			}
		}

		temp.push_back(cookie);
	}

	return temp;
}

Cookies::CookieProp::CookieProp(const std::pair<std::string, std::string>& init) {
	*this = init;
}

Cookies::CookieProp::CookieProp(const std::string& init) {
	this->first = init;
}

/*
std::string Cookies::stringify() const {
	
	std::string temp;

	for (const auto& entry : this->m_data) {
		if (temp.size()) temp += "; ";
		temp += entry.first + "=" + entry.second.at(0);
	}

	return temp;
}
*/