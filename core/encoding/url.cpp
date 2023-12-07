#include "../encoding.hpp"

#include <set>

using namespace Encoding;

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
			result.append(encodeHexByte(symbol).string);
			continue;
		}

		result.push_back(symbol);
	}

	return std::move(result);
}

std::string Encoding::decodeURIComponent(const std::string& input) {

	std::string result;
	result.reserve(input.size());

	for (size_t i = 0; i < input.size(); i++) {
		
		if (input[i] == '%') {

			HexByte temp;
			strncpy(temp.string, input.substr(i + 1, 2).c_str(), sizeof(temp.string));
			result.push_back(decodeHexByte(temp));

			i += 2;
			continue;
		}

		result.push_back(input[i]);
	}

	return std::move(result);
}
