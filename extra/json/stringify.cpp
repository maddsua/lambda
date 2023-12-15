#include "./json.hpp"

#include <map>
#include <stdexcept>

using namespace Lambda;
using namespace Lambda::JSON;

static const std::map<char, std::string> stringEscapeTable = {
	{ '\n', "\\\\n" },
	{ '\r', "\\\\r" },
	{ '\t', "\\\\t" },
	{ '\"', "\\\"" }
};

std::string JSON::stringify(const Property& data) {

	switch (data.type()) {

		case JSTypes::Integer:
			return std::to_string(data.asInt());

		case JSTypes::Float:
			return std::to_string(data.asFloat());

		case JSTypes::Boolean:
			return data.asBool() ? "true" : "false";

		case JSTypes::String: {

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

		case JSTypes::Null:
			return "null";

		case JSTypes::Array: {

			std::string temp;

			for (const auto& entry : data.asArray()) {
				if (temp.size()) temp.push_back(',');
				temp.append(stringify(entry));
			}

			return '[' + temp + ']';
		}

		case JSTypes::Map: {

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
