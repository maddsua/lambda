#include "../http.hpp"
#include "../strings.hpp"

using namespace HTTP;
using namespace Strings;

URLSearchParams::URLSearchParams(const std::string& urlString) {

	auto qmark = urlString.find_last_of('?');
	auto queries = split(qmark != std::string::npos ? urlString.substr(qmark + 1) : urlString, "&");
	
	for (const auto& item : queries) {

		if (item.find('=') == std::string::npos) continue;

		auto query = split(item, "=");
		if (query.size() < 2) continue;

		const auto key = query.at(0);
		const auto value = query.at(1);

		this->internalContent.push_back({
			toLowerCase(trim(key)),
			trim(value)
		});
	}
}

std::string URLSearchParams::stringify() const {
	auto result = std::string();
	for (const auto& item : this->internalContent) {
		result += (result.size() ? "&" : "") + item.key + "=" + item.value;
	}
	return result;
}
