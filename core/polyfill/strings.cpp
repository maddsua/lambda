#include "./polyfill.hpp"

#include <cstring>
#include <set>

using namespace Lambda;

void Strings::toLowerCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'Z' || c < 'A') continue;
		c += 0x20;
	}
}
std::string Strings::toLowerCase(const std::string& str) {
	auto temp = str;
	toLowerCase(temp);
	return temp;
}

void Strings::toUpperCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'z' || c < 'a') continue;
		c -= 0x20;
	}
}
std::string Strings::toUpperCase(const std::string& str) {
	auto temp = str;
	toUpperCase(temp);
	return temp;
}

void Strings::toTittleCase(std::string& str) {
	bool needsToBeCapital = true;
	for (auto& c : str) {
		if (needsToBeCapital && (c >= 'a' && c <= 'z')) c -= 0x20;
		else if (!needsToBeCapital && (c >= 'A' && c <= 'Z')) c += 0x20;
		needsToBeCapital = (c == ' ' || c == '-');
	}
}
std::string Strings::toTittleCase(const std::string& str) {
	auto temp = str;
	toTittleCase(temp);
	return temp;
}

bool Strings::includes(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
}
bool Strings::includes(const std::string& str, char token) {
	return str.find(token) != std::string::npos;
}
bool Strings::includes(const std::string& str, const std::vector <std::string>& substrs) {
	for (auto& item : substrs) {
		if (str.find(item) != std::string::npos)
			return true;
	}
	return false;
}

bool Strings::endsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == (str.size() - substr.size());
}
bool Strings::startsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == 0;
}

void Strings::trim(std::string& str) {

	//	list of characters to remove
	static const std::set<char> whitespaceChars = { '\r', '\n', '\t', ' ' };

	//	forward pass
	size_t pos_from = 0;
	while (pos_from < str.size()) {
		if (!whitespaceChars.contains(str[pos_from])) break;
		pos_from++;
	}

	//	backward pass
	const size_t endIdx = str.size() - 1;
	size_t pos_to = endIdx;
	while (pos_to > 0) {
		if (!whitespaceChars.contains(str[pos_to])) break;
		pos_to--;
	}

	if (pos_from == 0 && pos_to == endIdx) return;

	str = str.substr(pos_from, pos_to - (pos_from - 1));
}

std::string Strings::trim(const std::string& str) {
	auto temp = str;
	trim(temp);
	return temp;
}

std::vector<std::string> Strings::split(const std::string& str, const std::string& token) {

	std::vector <std::string> result;

	//	abort if source is empty
	if (!str.size()) return {};
	//	return entrire source if deliminator/token is empty
	if (!token.size()) return { str };

	//	return entrire source is deliminator/token is not present in the source
	auto match = str.find(token);
	if (match == std::string::npos) return { str };

	size_t startpos = 0;
	while (match != std::string::npos) {
		result.push_back(str.substr(startpos, match - startpos));
		startpos = match + token.size();
		match = str.find(token, startpos);
	}
	
	//	push the remaining part
	if (str.size() - startpos) result.push_back(str.substr(startpos));

	return result;
}

std::vector<std::string> Strings::split(const std::string& str, char separator) {

	std::vector <std::string> result;

	//	abort if source is empty
	if (!str.size()) return {};

	//	return entrire source is deliminator/token is not present in the source
	auto match = str.find(separator);
	if (match == std::string::npos) return { str };

	size_t startpos = 0;
	while (match != std::string::npos) {
		result.push_back(str.substr(startpos, match - startpos));
		startpos = match + 1;
		match = str.find(separator, startpos);
	}
	
	//	push the remaining part
	if (str.size() - startpos) result.push_back(str.substr(startpos));

	return result;
}

std::string Strings::join(const std::vector<std::string>& strs, const char* token) {

	std::string result;

	for(auto& str : strs) {
		if (result.size() > 0) result += token;
		result += str;
	}

	return result;
}

void Strings::replace(std::string& base, const std::string& sub, const std::string& replacement) {
	auto index = base.find(sub);
	if (index == std::string::npos) return;
	base.replace(index, sub.size(), replacement);
}

std::string Strings::replace(const std::string& base, const std::string& sub, const std::string& replacement) {
	auto temp = base;
	replace(temp, sub, replacement);
	return temp;
}

void Strings::replaceAll(std::string& base, const std::string& sub, const std::string& replacement) {
	size_t index = base.find(sub);
	while (index != std::string::npos) {
		base.replace(index, sub.size(), replacement);
		index += replacement.size();
		index = base.find(sub, index);
	}
}

std::string Strings::replaceAll(const std::string& base, const std::string& sub, const std::string& replacement) {
	auto temp = base;
	replaceAll(temp, sub, replacement);
	return temp;
}
