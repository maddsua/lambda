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
