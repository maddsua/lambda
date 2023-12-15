#include "./json.hpp"
#include "../../core/polyfill/polyfill.hpp"

#include <array>
#include <stdexcept>

using namespace Lambda;
using namespace Lambda::JSON;

static const std::array<std::pair<std::string, std::string>, 4> stringUnescapeTable = {
	std::make_pair("\\\\n", "\n"),
	std::make_pair("\\\\r", "\r"),
	std::make_pair("\\\\t", "\t"),
	std::make_pair("\\\"", "\"")
};

Property JSON::parse(const std::string& text) {

	auto objectText = Strings::trim(text);
	if (!objectText.size()) throw std::runtime_error("Invalid JSON: empty text");

	auto isArray = objectText.starts_with('[');

	if (!isArray && !objectText.starts_with('{')) {
		
		if (objectText.starts_with('\"')) {

			auto textcontent = objectText.substr(1, objectText.size() - 2);

			auto illegalQuotePos = textcontent.find('\"');

			if (illegalQuotePos != std::string::npos && (illegalQuotePos == 0 || textcontent.at(illegalQuotePos - 1) != '\\'))
				throw std::runtime_error("Invalid JSON: unescaped double quote");

			for (const auto& symbol : stringUnescapeTable)
				Strings::replaceAll(textcontent, symbol.first, symbol.second);

			return Property(textcontent);

		} else if (isdigit(objectText.at(0))) {
			return objectText.find('.') == std::string::npos ? Property(std::stoi(objectText)) : Property(std::stold(objectText));
		} else if ((objectText.starts_with('t') || objectText.starts_with('f')) && objectText.ends_with('e')) {
			return Property(objectText == "true");
		} else {
			return objectText == "null" ? Property(nullptr) : Property();
		}
	}

	std::vector<std::string> entries;

	//	split this shit into segments between commas
	size_t ptnss_object = 0;
	size_t ptnss_array = 0;
	bool ptnss_string = 0;

	size_t segmentStart = 1;

	for (size_t i = segmentStart; i < objectText.size() - 1; i++) {

		switch (objectText[i]) {
			case '{':
				ptnss_object++;
				break;
			case '}':
				ptnss_object--;
				break;
			case '[':
				ptnss_array++;
				break;
			case ']':
				ptnss_array--;
				break;
			case '\"':
				ptnss_string = !ptnss_string;
				break;
			default: break;
		}

		if (!ptnss_object && !ptnss_array && !ptnss_string && objectText[i] == ',') {
			entries.push_back(objectText.substr(segmentStart, i - segmentStart));
			segmentStart = i + 1;
		}
	}

	entries.push_back(objectText.substr(segmentStart, objectText.size() - segmentStart - 1));

	if (isArray) {

		std::vector<Property> temp;

		for (const auto& entry : entries) {
			auto entryTrimmed = Strings::trim(entry);
			if (!entryTrimmed.size()) throw std::runtime_error("Invalid JSON: trailing comma has been found");
			temp.push_back(parse(entryTrimmed));
		}

		return Property(temp);

	} else {

		std::unordered_map<std::string, Property> temp;

		for (const auto& entry : entries) {

			auto entryTrimmed = Strings::trim(entry);
			if (!entryTrimmed.size()) throw std::runtime_error("Invalid JSON: trailing comma has been found");

			auto keyEnded = std::string::npos;
			bool insideString = false;

			for (size_t i = 0; i < entryTrimmed.size(); i++) {
				if (entryTrimmed[i] == '\"') {
					insideString = !insideString;
				} else if (!insideString && entryTrimmed[i] == ':') {
					keyEnded = i;
					break;
				}
			}

			if (keyEnded == std::string::npos) throw std::runtime_error("Invalid JSON: no separator after property name");

			auto key = Strings::trim(entryTrimmed.substr(0, keyEnded));
			auto value = Strings::trim(entryTrimmed.substr(keyEnded + 1));
			auto useKey = key.starts_with('\"') ? key.substr(1, key.size() - 2) : key;

			temp[useKey] = parse(value);
		}

		return Property(temp);
	}
}
