#include "../http.hpp"
#include "../polyfill.hpp"

using namespace Lambda::HTTP;
using namespace Lambda::Strings;

Headers::Headers(const std::vector<KVpair>& init) {
	this->internalContent = init;
}

void Headers::append(const std::string& key, const std::string value) {
	this->internalContent.push_back({ toLowerCase(key), value });
}

std::vector<std::string> Headers::getAll(const std::string& key) const {
	const auto keyNormalized = toLowerCase(key);
	std::vector<std::string> result;
	for (const auto& item : this->internalContent) {
		if (item.key == keyNormalized)
			result.push_back(item.value);
	}
	return result;
}
