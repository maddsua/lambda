#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda::HTTP;

Cookies::Cookies(const std::initializer_list<KVpair>& init) {
	this->mergeInitList(init);
}

Cookies::Cookies(const std::string& cookies) {

	auto entries = Strings::split(cookies, "; ");

	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = Strings::trim(entry.substr(0, kvSeparatorIdx));
		auto value = Strings::trim(entry.substr(kvSeparatorIdx + 1));

		if (!key.size() || !value.size()) continue;

		this->m_data[key] = { value };
	}
}

std::string Cookies::stringify() const {
	
	std::string temp;

	for (const auto& entry : this->m_data) {
		if (temp.size()) temp += "; ";
		temp += entry.first + "=" + entry.second.at(0);
	}

	return temp;
}
