#include "./core.hpp"
#include <stdexcept>

#include <iostream>

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

/**
 * Just a memo of how URL schema looks
 * https://user:password@example.com:443/document?search=query#fragment
*/

URL::URL(const std::string& href) {

	//	alright so let's start with getting url schema. that's the "http" thing
	auto cursor = href.find("://");
	if (cursor == std::string::npos) {
		throw std::runtime_error("Can't determine url schema");
	}

	this->protocol = href.substr(0, cursor);
	cursor += 3;

	std::string addrString;

	//	find separator between hostname and stuff and document path and it's stuff
	const auto docStart = href.find_first_of('/', cursor);
	if (docStart != std::string::npos) {

		addrString = href.substr(cursor, docStart - cursor);

		auto docString = href.substr(docStart);

		//	get document fragment aka hash
		cursor = docString.find_first_of('#');
		if (cursor != std::string::npos) {
			this->hash = docString.substr(cursor);
			docString = docString.substr(0, cursor);
		}

		//	now get search query
		cursor = docString.find_first_of('?');
		if (cursor != std::string::npos) {
			this->searchParams = URLSearchParams(docString.substr(cursor));
			docString = docString.substr(0, cursor);
		}

		this->pathname = docString;
	}
	else {
		this->pathname = '/';
		addrString = href.substr(cursor);
	}

	// get http auth sorted out
	cursor = addrString.find('@');
	if (cursor != std::string::npos) {

		auto credentails = addrString.substr(0, cursor);
		const auto credSep = credentails.find(':');
		if (credSep != std::string::npos) {
			this->username = credentails.substr(0, credSep);
			this->password = credentails.substr(credSep + 1);
		}

		addrString = addrString.substr(cursor + 1);
	}

	this->host = addrString;

	//	get hostname and port
	cursor = addrString.find(':');
	if (cursor != std::string::npos) {
		this->port = addrString.substr(cursor + 1);
		this->hostname = addrString.substr(0, cursor);
	}
	else {
		this->hostname = addrString;
		this->port = "80";
	}
}
