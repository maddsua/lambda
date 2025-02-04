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

std::string Values::m_format_key(const std::string& key) const noexcept {

	//	forward pass
	size_t trim_from = 0;
	while (trim_from < key.size() && std::isspace(key[trim_from])) {
		trim_from++;
	}

	//	backward pass
	size_t trim_to = key.size() - 1;
	while (trim_to > 0 && std::isspace(key[trim_to])) {
		trim_to--;
	}

	auto form_key = key.substr(trim_from, trim_to - (trim_from - 1));

	//	reset to lower case
	for (auto& rune : form_key) {
		if (rune >= 'A' && rune <= 'Z') {
			rune += 0x20;
		}
	}

	return form_key;
}

bool Values::has(const std::string& key) const noexcept {

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
		return false;
	}

	return this->m_entries.find(normalized_key) != this->m_entries.end();
}

std::string Values::get(const std::string& key) const noexcept {

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
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

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
		return {};
	}

	auto entry = this->m_entries.find(normalized_key);
	if (entry == this->m_entries.end()) {
		return {};
	}

	if (!entry->second.size()) {
		return {};
	}

	return entry->second;
}

void Values::set(const std::string& key, const std::string& value) noexcept {

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
		return;
	}

	this->m_entries[normalized_key] = { value };
}

void Values::append(const std::string& key, const std::string& value) noexcept {

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
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

	auto normalized_key = this->m_format_key(key);
	if (normalized_key == "") {
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

std::string HTTP::reset_case(std::string value) {
	
	for (auto& rune : value) {
		if (rune >= 'A' && rune <= 'Z') {
			rune += 0x20;
		}
	}
	
	return value;
}
