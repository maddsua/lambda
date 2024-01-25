#include "./encoding.hpp"

#include <set>
#include <cstring>

using namespace Lambda;

static const auto escapeChars = std::set<char>({
	'\"', '\'', '\\', '/', '>', '<', ' ', '%', '{', '}',
	'|', '^', '`', ':', '\?', '#', '[', ']', '@', '!',
	'$', '&', '(', ')', '*', '+', ',', ';', '='
});

std::string Encoding::encodeURIComponent(const std::string& input) {
	
	std::string result;
	result.reserve(input.size());

	for (auto symbol : input) {

		if (escapeChars.contains(symbol)) {
			result.push_back('%');
			result.append(byteToHex(symbol).string);
			continue;
		}

		result.push_back(symbol);
	}

	return result;
}

std::string Encoding::decodeURIComponent(const std::string& input) {

	std::string result;
	result.reserve(input.size());

	for (size_t i = 0; i < input.size(); i++) {
		
		if (input[i] == '%') {

			HexByte temp;
			strncpy(temp.string, input.substr(i + 1, 2).c_str(), sizeof(temp.string));
			result.push_back(hexToByte(temp));

			i += 2;
			continue;
		}

		result.push_back(input[i]);
	}

	return result;
}
