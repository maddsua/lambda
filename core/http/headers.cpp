#include "../core.hpp"

using namespace HTTP;
using namespace Strings;

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
