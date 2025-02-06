#include "./http.hpp"
#include "./http_utils.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Values::Values(const Values& other) {
	this->m_entries = other.m_entries;
}

Values::Values(Values&& other) {
	this->m_entries = std::move(other.m_entries);
}

Values& Values::operator=(const Values& other) noexcept {
	this->m_entries = other.m_entries;
	return *this;
}

Values& Values::operator=(Values&& other) noexcept {
	this->m_entries = std::move(other.m_entries);
	return *this;
}

bool Values::has(const std::string& key) const noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return false;
	}

	return this->m_entries.find(normalized_key) != this->m_entries.end();
}

std::string Values::get(const std::string& key) const noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return "";
	}

	auto entry = this->m_entries.find(normalized_key);
	if (entry == this->m_entries.end()) {
		return "";
	}

	auto values_size = entry->second.size();
	if (!values_size) {
		return "";
	}

	return entry->second[values_size - 1];
}

MultiValue Values::get_all(const std::string& key) const noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return {};
	}

	auto entry = this->m_entries.find(normalized_key);
	if (entry == this->m_entries.end()) {
		return {};
	}

	if (entry->second.empty()) {
		return {};
	}

	return entry->second;
}

void Values::set(const std::string& key, const std::string& value) noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return;
	}

	this->m_entries[normalized_key] = { value };
}

void Values::append(const std::string& key, const std::string& value) noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return;
	}

	auto entry = this->m_entries.find(normalized_key);
	if (entry == this->m_entries.end()) {
		this->m_entries[normalized_key] = { value };
		return;
	}

	entry->second.push_back(value);
}

void Values::del(const std::string& key) noexcept {

	auto normalized_key = HTTP::reset_case(key);
	if (normalized_key.empty()) {
		return;
	}

	this->m_entries.erase(normalized_key);
}

Entries Values::entries() const noexcept {

	Entries entries;

	for (const auto& entry : this->m_entries) {
		for (const auto& value : entry.second) {
			entries.push_back({ entry.first, value });
		}
	}

	return entries;
}

size_t Values::size() const noexcept {
	return this->m_entries.size();
}

bool Values::empty() const noexcept {
	return this->m_entries.empty();
}

std::string HTTP::reset_case(std::string token) {

	//	find token beginning
	size_t trim_begin = 0;
	while (trim_begin < token.size() && std::isspace(token[trim_begin])) {
		trim_begin++;
	}

	//	find token end
	size_t value_end = token.size() - 1;
	size_t trim_end = value_end;
	while (trim_end > 0 && std::isspace(token[trim_end])) {
		trim_end--;
	}

	//	trim spaces
	if (trim_begin > 0 || trim_begin < value_end) {
		token = token.substr(trim_begin, trim_end - (trim_begin - 1));
	}

	//	reset to lowercase
	for (auto& rune : token) {
		if (rune >= 'A' && rune <= 'Z') {
			rune += 0x20;
		}
	}
	
	return token;
}
