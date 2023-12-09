#include "../json.hpp"

#include <map>
#include <stdexcept>

using namespace JSON;

static const std::map<char, std::string> stringEscapeTable = {
	{ '\n', "\\\\n" },
	{ '\r', "\\\\r" },
	{ '\t', "\\\\t" },
	{ '\"', "\\\"" }
};

std::string JSON::stringify(const Property& data) {

	switch (data.type()) {

		case JSTypes::Type_Integer:
			return std::to_string(data.asInt());

		case JSTypes::Type_Float:
			return std::to_string(data.asFloat());

		case JSTypes::Type_Boolean:
			return data.asBool() ? "true" : "false";

		case JSTypes::Type_String: {

			auto textcontent = data.asString();
			std::string temp;
			temp.reserve(textcontent.size());

			for (auto symbol : textcontent) {

				auto escaped = stringEscapeTable.find(symbol);
				if (escaped != stringEscapeTable.end()) {
					temp.append(escaped->second);
					continue;
				}

				temp.push_back(symbol);
			}

			return '\"' + temp + '\"';
		}

		case JSTypes::Type_Null:
			return "null";

		case JSTypes::Type_Array: {

			std::string temp;

			for (const auto& entry : data.asArray()) {
				if (temp.size()) temp.push_back(',');
				temp.append(stringify(entry));
			}

			return '[' + temp + ']';
		}

		case JSTypes::Type_Map: {

			std::string temp;

			for (const auto& entry : data.asMap()) {
				if (temp.size()) temp.push_back(',');
				temp.append('\"' + entry.first + "\":");
				temp.append(stringify(entry.second));
			}

			return '{' + temp + '}';
		}

		default: break;
	}

	return "undefined";
}
