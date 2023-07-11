#include "./http.hpp"
#include <string.h>
#include <map>
#include <set>

void Lambda::HTTP::stringToLowerCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'Z' || c < 'A') continue;
		c += 0x20;
	}
}
std::string Lambda::HTTP::stringToLowerCase(const std::string& str) {
	auto temp = str;
	stringToLowerCase(temp);
	return std::move(temp);
}

void Lambda::HTTP::stringToUpperCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'z' || c < 'a') continue;
		c -= 0x20;
	}
}
std::string Lambda::HTTP::stringToUpperCase(const std::string& str) {
	auto temp = str;
	stringToUpperCase(temp);
	return std::move(temp);
}

void Lambda::HTTP::stringToTittleCase(std::string& str) {
	bool needToBeCapital = true;
	for (auto& c : str) {
		if (needToBeCapital && (c >= 'a' && c <= 'z')) c -= 0x20;
		if (!needToBeCapital && (c >= 'A' && c <= 'Z')) c += 0x20;
		needToBeCapital = (c == ' ' || c == '-');
	}
}
std::string Lambda::HTTP::stringToTittleCase(const std::string& str) {
	auto temp = str;
	stringToTittleCase(temp);
	return std::move(temp);
}

bool Lambda::HTTP::stringIncludes(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
}
bool Lambda::HTTP::stringIncludes(const std::string& str, const std::vector <std::string>& substrs) {
	for (auto& item : substrs) {
		if (str.find(item) != std::string::npos)
			return true;
	}
	return false;
}

bool Lambda::HTTP::stringEndsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == (str.size() - substr.size());
}
bool Lambda::HTTP::stringStartsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == 0;
}

void Lambda::HTTP::stringTrim(std::string& str) {

	//	list of characters to remove
	static const std::set<char> whitespaceChars = {'\r','\n','\t',' '};

	//	forward pass
	size_t pos_from = 0;
	while (pos_from < str.size()) {
		if (whitespaceChars.find(str[pos_from]) == whitespaceChars.end()) break;
		pos_from++;
	}
	
	//	backward pass
	const size_t endIdx = str.size() - 1;
	size_t pos_to = endIdx;
	while (pos_to >= 0) {
		if (whitespaceChars.find(str[pos_to]) == whitespaceChars.end()) break;
		pos_to--;
	}

	if (pos_from == 0 && pos_to == endIdx) return;
	
	str = str.substr(pos_from, pos_to - (pos_from - 1));
}
std::string Lambda::HTTP::stringTrim(const std::string& str) {
	auto temp = str;
	stringTrim(temp);
	return std::move(temp);
}

std::string Lambda::HTTP::stringJoin(const std::vector<std::string>& strs, const char* token) {
	std::string result;
	for(auto& str : strs) {
		if (result.size() > 0) result += token;
		result += str;
	}
	return std::move(result);
}

std::vector<std::string> Lambda::HTTP::stringSplit(const std::string& str, const char* token) {

	std::vector <std::string> result;

	//	abort if source is empty
	if (!str.size()) return {};
	//	return entrire source if deliminator/token is empty
	auto tokenLength = strlen(token);
	if (!tokenLength) return { str };

	//	return entrire source is deliminator/token is not present in the source
	auto match = str.find(token);
	if (match == std::string::npos) return { str };

	size_t startpos = 0;
	while (match != std::string::npos) {
		result.push_back(str.substr(startpos, match - startpos));
		startpos = match + tokenLength;
		match = str.find(token, startpos);
	}
	
	//	push the remaining part
	if (str.size() - startpos) result.push_back(str.substr(startpos));

	return std::move(result);
}
