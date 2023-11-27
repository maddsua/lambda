#include "./core.hpp"
#include <string.h>


void Strings::toLowerCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'Z' || c < 'A') continue;
		c += 0x20;
	}
}
std::string Strings::toLowerCase(const std::string& str) {
	auto temp = str;
	toLowerCase(temp);
	return std::move(temp);
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
	return std::move(temp);
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
	return std::move(temp);
}

bool Strings::includes(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
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
	static const std::string whitespaceChars = "\r\n\t ";

	//	forward pass
	size_t pos_from = 0;
	bool spaceCharMatched = false;
	while (pos_from < str.size()) {
		if (whitespaceChars.find(str[pos_from]) == std::string::npos) break;
		pos_from++;
	}

	//	backward pass
	const size_t endIdx = str.size() - 1;
	size_t pos_to = endIdx;
	while (pos_to >= 0) {
		if (whitespaceChars.find(str[pos_to]) == std::string::npos) break;
		pos_to--;
	}

	if (pos_from == 0 && pos_to == endIdx) return;

	str = str.substr(pos_from, pos_to - (pos_from - 1));
}

std::string Strings::trim(const std::string& str) {
	auto temp = str;
	trim(temp);
	return std::move(temp);
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

	return std::move(result);
}
