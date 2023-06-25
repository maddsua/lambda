#include "http.hpp"
#include <string.h>
#include <map>

void HTTP::stringToLowerCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'Z' || c < 'A') continue;
		c += 0x20;
	}
}
std::string HTTP::stringToLowerCase(const std::string& str) {
	auto temp = str;
	stringToLowerCase(temp);
	return temp;
}

void HTTP::stringToUpperCase(std::string& str) {
	for (auto& c : str) {
		if (c > 'z' || c < 'a') continue;
		c -= 0x20;
	}
}
std::string HTTP::stringToUpperCase(const std::string& str) {
	auto temp = str;
	stringToUpperCase(temp);
	return temp;
}

void HTTP::stringToTittleCase(std::string& str) {
	bool needToBeCapital = true;
	for (auto& c : str) {
		if (needToBeCapital && (c >= 'a' && c <= 'z')) c -= 0x20;
		if (!needToBeCapital && (c >= 'A' && c <= 'Z')) c += 0x20;
		needToBeCapital = (c == ' ' || c == '-');
	}
}
std::string HTTP::stringToTittleCase(const std::string& str) {
	auto temp = str;
	stringToTittleCase(temp);
	return temp;
}

bool HTTP::stringIncludes(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
}
bool HTTP::stringIncludes(const std::string& str, const std::vector <std::string>& substrs) {
	for (auto& item : substrs) {
		if (str.find(item) != std::string::npos)
			return true;
	}
	return false;
}

bool HTTP::stringEndsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == (str.size() - substr.size());
}
bool HTTP::stringStartsWith(const std::string& str, const std::string& substr) {
	return str.find(substr) == 0;
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
std::string HTTP::stringTrim(const std::string& str) {
	auto temp = str;
	stringTrim(temp);
	return temp;
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

typedef std::map<char, std::string> URLEncodeMap;
const auto URLEncodeTable = URLEncodeMap({
	{'\"', "%22"},
	{'\'', "%27"},
	{'\\', "%5C"},
	{'/', "%2F"},
	{'>', "%3E"},
	{'<', "%3C"},
	{' ', "%20"},
	{'%', "%25"},
	{'{', "%7B"},
	{'}', "%7D"},
	{'|', "%7C"},
	{'^', "%5E"},
	{'`', "%60"},
	{':', "%3A"},
	{'\?', "%3F"},
	{'#', "%23"},
	{'[', "%5B"},
	{']', "%5D"},
	{'@', "%40"},
	{'!', "%21"},
	{'$', "%24"},
	{'&', "%26"},
	{'(', "%28"},
	{')', "%29"},
	{'*', "%2A"},
	{'+', "%2B"},
	{',', "%2C"},
	{';', "%3B"},
	{'=', "%3D"}
});

std::string HTTP::encodeURIComponent(const std::string& str) {
	auto result = std::string();
	for (auto c : str) {
		if (URLEncodeTable.find(c) != URLEncodeTable.end()) {
			result.append(URLEncodeTable.at(c));
			continue;
		}
		result.push_back(c);
	}
	return result;
}
