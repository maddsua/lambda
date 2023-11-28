#include "../core.hpp"

using namespace HTTP;
using namespace Strings;

KVContainer::KVContainer(const std::vector<KVpair>& entries) {
	this->internalContent = entries;
}

bool KVContainer::has(const std::string& key) const {
	const auto keyNormalized = toLowerCase(key);
	for (const auto& item : this->internalContent) {
		if (item.key == keyNormalized) return true;
	}
	return false;
}

std::string KVContainer::get(const std::string& key) const {
	const auto keyNormalized = toLowerCase(key);
	for (const auto& item : this->internalContent) {
		if (item.key == keyNormalized) return item.value;
	}
	return {};
}

void KVContainer::delNormalized(const std::string& keyNormalized) {
	for (size_t i = 0; i < this->internalContent.size(); i++) {
		if (this->internalContent.at(i).key == keyNormalized) {
			this->internalContent.erase(this->internalContent.begin() + i);
			i--;
		}
	}
}

void KVContainer::del(const std::string& key) {
	const auto keyNormalized = toLowerCase(key);
	this->delNormalized(keyNormalized);
}

void KVContainer::set(const std::string& key, const std::string value) {
	const auto keyNormalized = toLowerCase(key);
	this->delNormalized(keyNormalized);
	this->internalContent.push_back({ keyNormalized, value });
}

const std::vector<KVpair>& KVContainer::entries() const {
	return this->internalContent;
}
