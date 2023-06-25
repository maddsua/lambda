#include "http.hpp"

#include <string.h>

void HTTP::stringToLowerCase(std::string& str) {
	for (auto &&c : str) {
		if (c > 'Z' || c < 'A') continue;
		c += 0x20;
	}
}

void HTTP::stringToUpperCase(std::string& str) {
	for (auto &&c : str) {
		if (c > 'z' || c < 'a') continue;
		c -= 0x20;
	}
}

void HTTP::stringToTittleCase(std::string& str) {
	bool needToCapitalize = true;
	for (auto &&c : str) {
		if (needToCapitalize && (c >= 'a' && c <= 'z')) c -= 0x20;
		needToCapitalize = (c == ' ' || c == '-');
	}
}

void HTTP::stringTrim(std::string& str) {

	//	list of characters to remove
	static const std::string whitespaceChars = "\r\n\t ";

	//	do both passes at the same time
	size_t pos_from = std::string::npos;
	size_t pos_to = std::string::npos;

	for (size_t i = 0; i < str.length(); i++) {

		auto found = whitespaceChars.find(str[i]) != std::string::npos;

		if (pos_from == std::string::npos && !found) {
			pos_from = i;
		}

		if (pos_from != std::string::npos && found) {
			pos_to = i;
			break;
		}
	}
	
	if (pos_from == std::string::npos || pos_to == std::string::npos) return;
	
	str = str.substr(pos_from, pos_to - pos_from);
}

std::vector<std::string> HTTP::stringSplit(const std::string& str, const char* token) {

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

	return result;
}
