#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

URLSearchParams::URLSearchParams(const std::string& queries) {

	auto qmark = queries.find_last_of('?');
	auto entries = Strings::split(qmark != std::string::npos ? queries.substr(qmark + 1) : queries, '&');
	
	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = Strings::trim(entry.substr(0, kvSeparatorIdx));
		auto value = Strings::trim(entry.substr(kvSeparatorIdx + 1));

		if (!key.size() || !value.size()) continue;

		this->m_data[key] = { value };
	}
}

std::string URLSearchParams::stringify() const {
	auto result = std::string();
	for (const auto& entry : this->m_data) {
		result += (result.size() ? "&" : "") + entry.first + "=" + entry.second.at(0);
	}
	return result;
}
