#include "./core.hpp"
#include <stdexcept>

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


URLSearchParams::URLSearchParams(const std::string& URLString) {

	auto qmark = URLString.find_last_of('?');
	auto queries = split(qmark != std::string::npos ? URLString.substr(qmark + 1) : URLString, "&");
	
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

URL::URL(const std::string& href) {

	auto temp = href;
	auto pos = std::string::npos;

	//	start parsing from the end
	//	get document fragment aka hash
	pos = temp.find_first_of('#');
	if (pos != std::string::npos) {
		this->hash = temp.substr(pos);
		temp = temp.substr(0, pos);
	}

	//	now get search query
	pos = temp.find_first_of('?');
	if (pos != std::string::npos) {
		this->searchParams = URLSearchParams(temp.substr(pos));
		temp = temp.substr(0, pos);
	}

	//	then jump to the beginning to get protocol/schema
	pos = temp.find("://");
	if (pos != std::string::npos) {
		this->protocol = temp.substr(0, pos);
		temp = temp.substr(pos + 3);
	}

	//	get host
	pos = temp.find_first_of('/');
	if (pos == std::string::npos) throw new std::runtime_error("Cannot parse URL: path start undefined");

	this->host = temp.substr(0, pos);
	temp = temp.substr(pos);

	//	split host to port and hostname real quick
	pos = this->host.find_first_of(':');
	if (pos != std::string::npos) {
		this->port = this->host.substr(pos + 1);
		this->hostname = this->host.substr(0, pos);
	}
	else {
		this->hostname = this->host;
	}

	//	we're basically left with pathname
	this->pathname = temp;
}
