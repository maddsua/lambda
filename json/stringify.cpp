#include "./json.hpp"

#include <map>
#include <stdexcept>

using namespace Lambda;
using namespace Lambda::JSON;

std::string rune_replace(char rune) {
	switch (rune) {
		case '\n': return "\\\\n";
		case '\e': return "\\\\r";
		case '\t': return "\\\\t";
		case '\"': return "\\\"";
		default: return "";
	}
}

std::string JSON::stringify(const Property& value) {

	switch (value.type()) {

		case Type::Integer:
			return std::to_string(value.as_int());

		case Type::Float: {

			auto token = std::to_string(value.as_float());

			size_t decimal_begin = std::string::npos;
			for (size_t idx = 0; idx < token.size(); idx++) {
				if (token[idx] == '.') {
					decimal_begin = idx;
					break;
				}
			}

			if (decimal_begin == std::string::npos) {
				return token;
			}
			
			size_t zeros_end = std::string::npos;
			for (int64_t idx = token.size() - 1; idx >= static_cast<int64_t>(decimal_begin); idx--) {
				if (token[idx] != '0') {
					zeros_end = idx + 1;
					break;
				}
			}
			
			if (zeros_end == std::string::npos) {
				return token;
			}

			if (zeros_end - decimal_begin > 1) {
				return token.substr(0, zeros_end);
			}

			return token.substr(0, decimal_begin);	
		}

		case Type::Boolean:
			return value.as_bool() ? "true" : "false";

		case Type::String: {

			auto content = value.as_string();
			std::string value;
			value.reserve(content.size());

			for (auto rune : content) {

				auto escaped = rune_replace(rune);
				if (escaped.size()) {
					value.append(escaped);
					continue;
				}

				value.push_back(rune);
			}

			return '\"' + value + '\"';
		}

		case Type::Array: {

			std::string buff;

			for (const auto& entry : value.as_array()) {
				
				if (buff.size()) {
					buff.push_back(',');
				}

				buff.append(stringify(entry));
			}

			return '[' + buff + ']';
		}

		case Type::Object: {

			std::string buff;

			for (const auto& entry : value.as_map()) {
				
				if (buff.size()) {
					buff.push_back(',');
				}

				buff.append('\"' + entry.first + "\":");
				buff.append(stringify(entry.second));
			}

			return '{' + buff + '}';
		}

		default: break;
	}

	return "null";
}

std::string JSON::stringify(const std::string& value) {
	return stringify(Property(value));
}

std::string JSON::stringify(const char* value) {
	return stringify(Property(value));
}

std::string JSON::stringify(uint16_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(int16_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(uint32_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(int32_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(uint64_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(int64_t value) {
	return stringify(Property(value));
}

std::string JSON::stringify(float value) {
	return stringify(Property(value));
}

std::string JSON::stringify(double value) {
	return stringify(Property(value));
}

std::string JSON::stringify(long double value) {
	return stringify(Property(value));
}

std::string JSON::stringify(Object value) {
	return stringify(Property(value));
}

std::string JSON::stringify(Array value) {
	return stringify(Property(value));
}
