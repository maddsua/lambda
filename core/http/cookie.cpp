#include "../core.hpp"

using namespace HTTP;
using namespace Strings;

Cookie::Cookie(const std::string& cookies) {

	auto entries = split(cookies, "; ");

	for (const auto& entry : entries) {

		auto kvSeparatorIdx = entry.find('=');
		if (kvSeparatorIdx == std::string::npos || kvSeparatorIdx == 0 || kvSeparatorIdx == entry.size() - 1) continue;

		auto key = entry.substr(0, kvSeparatorIdx);
		auto value = entry.substr(kvSeparatorIdx + 1);

		trim(key);
		trim(value);

		this->internalContent.push_back({ key, value });
	}
}

std::string Cookie::stringify() const {
	
	std::string temp;

	for (const auto& item : this->internalContent) {
		if (temp.size()) temp += "; ";
		temp += item.key + "=" + item.value;
	}

	return temp;
}
