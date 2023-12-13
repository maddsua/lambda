#include "../http.hpp"
#include "../polyfill.hpp"

using namespace Lambda::HTTP;

Cookie::Cookie(const std::string& cookies) {

	auto entries = Strings::split(cookies, "; ");

	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = Strings::trim(entry.substr(0, kvSeparatorIdx));
		auto value = Strings::trim(entry.substr(kvSeparatorIdx + 1));

		if (!key.size() || !value.size()) continue;

		this->data[key] = { value };
	}
}

std::string Cookie::stringify() const {
	
	std::string temp;

	for (const auto& entry : this->data) {
		if (temp.size()) temp += "; ";
		temp += entry.first + "=" + entry.second.at(0);
	}

	return temp;
}
