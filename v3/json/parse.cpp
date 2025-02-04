#include "./json.hpp"
#include "../../core/polyfill/polyfill.hpp"

#include <array>
#include <stdexcept>

using namespace Lambda;
using namespace Lambda::JSON;

Property parse_range(const std::string& data, size_t begin, size_t end);
Property parse_string(const std::string& data, size_t begin, size_t end);
Property parse_number(const std::string& data, size_t begin, size_t end);
Property parse_object(const std::string& data, size_t begin, size_t end);
Property parse_array(const std::string& data, size_t begin, size_t end);
Property parse_literal(const std::string& data, size_t begin, size_t end);

bool is_valid_escaped_quote(const std::string& data, size_t idx, size_t begin) {

	if (idx - begin < 3) {
		return false;
	}

	return data[idx - 1] == '\\' && data[idx - 2] == '\\';
}

Property JSON::parse(const std::string& data) {

	if (data.size() > UINT32_MAX) {
		throw std::runtime_error("Unable to parse JSON: buffer size too large (max UINT32_MAX; ~4GB)");
	}

	auto result = parse_range(data, 0, data.size());
	if (result.type() == Type::Undefined) {
		throw std::runtime_error("Invalid JSON: Empty input");
	}

	return result;
}

Property parse_range(const std::string& data, size_t begin, size_t end) {

	for (size_t idx = begin; idx < end; idx++) {
		if (!isspace(data[idx])) {
			begin = idx;
			break;
		}
	}

	for (int64_t idx = end - 1; idx >= static_cast<int64_t>(begin); idx--) {
		if (!isspace(data[idx])) {
			end = idx + 1;
			break;
		}
	}

	if (end - begin <= 0) {
		return Property();
	}

	auto first_rune = data[begin];

	if (first_rune == '"') {
		return parse_string(data, begin, end);
	} else if (isdigit(data[begin]) || first_rune == '-') {
		return parse_number(data, begin, end);
	} else if (first_rune == '{') {
		return parse_object(data, begin, end);
	} else if (first_rune == '[') {
		return parse_array(data, begin, end);
	} else {
		return parse_literal(data, begin, end);
	}

	throw std::runtime_error("Invalid JSON");
}

std::vector<std::pair<size_t, size_t>> get_object_entries(const std::string& data, size_t begin, size_t end) {

	auto entry_begin = begin;
	std::vector<std::pair<size_t, size_t>> entries;

	size_t scope_obj = 0;
	size_t scope_arr = 0;
	bool scope_str = 0;

	for (size_t idx = begin; idx < end; idx++) {

		switch (data[idx]) {

			case '{': {
				scope_obj++;
			} break;

			case '}': {
				scope_obj--;
			} break;

			case '[': {
				scope_arr++;
			} break;

			case ']': {
				scope_arr--;
			} break;

			case '\"': {

				if (!is_valid_escaped_quote(data, idx, begin)) {
					scope_str = !scope_str;
				}

			} break;

			default: break;
		}

		if (!scope_obj && !scope_arr && !scope_str && data[idx] == ',') {
			entries.push_back({ entry_begin, idx });
			entry_begin = idx + 1;
		}
	}

	entries.push_back({ entry_begin, end });

	return entries;
}

Property parse_string(const std::string& data, size_t begin, size_t end) {

	if (data[begin] != '"' || data[end - 1] != '"') {
		throw std::runtime_error("Invalid JSON: String property: Bounds invalid");
	}

	begin++;
	end--;

	for (size_t idx = begin; idx < end; idx++) {
		if (data[idx] == '"' && !is_valid_escaped_quote(data, idx, begin)) {
			throw std::runtime_error("Invalid JSON: String property: Unescaped double quote");
		}
	}
	
	std::string value;
	value.reserve(data.size());
	
	for (size_t idx = begin; idx < end; idx++) {

		auto rune = data[idx];
		
		if (rune == '\\') {
			
			auto have_next = end - idx;
			if (!have_next) {
				throw std::runtime_error("Invalid JSON: String property: Single backslash at the end of a string");
			}

			auto next_rune = data[idx + 1];
			if (next_rune != '\\') {
				throw std::runtime_error("Invalid JSON: String property: Backslash is not followed by an escaped character");
			}

			if (have_next < 2) {
				throw std::runtime_error("Invalid JSON: String property: Incomplete escape sequence at the end of a string");
			}

			auto after_next_rune = data[idx + 2];
			switch (after_next_rune) {
				case 'n': {
					value.push_back('\n');
				} break;
				case 'r': {
					value.push_back('\r');
				} break;
				case 't': {
					value.push_back('\t');
				} break;
				case '"': {
					value.push_back('"');
				} break;
				case '\\': {
					value.push_back('\\');
				} break;
				default:
					throw std::runtime_error("Invalid JSON: String property: Invalid escape sequence");
			}

			idx += 2;
			continue;
		}

		value.push_back(rune);
	}
	
	return Property(value);
}

Property parse_number(const std::string& data, size_t begin, size_t end) {
	
	for (size_t idx = 0; idx < end; idx++){
		if (data[idx] == '.') {
			return Property(std::stold(data.substr(begin, end - begin)));
		}
	}

	return Property(std::stol(data.substr(begin, end - begin)));
}

Property parse_object(const std::string& data, size_t begin, size_t end) {

	if (data[begin] != '{' || data[end - 1] != '}') {
		throw std::runtime_error("Invalid JSON: Object object: Invalid scope tokens");
	}

	Object entries;

	auto unwrap = [&](size_t begin, size_t end) {

		std::string key;
		size_t key_begin = std::string::npos;
		size_t value_begin = std::string::npos;

		for (size_t idx = begin; idx < end; idx++) {

			auto rune = data[idx];

			if (!key.size() && rune == '"' && !(idx > begin && data[idx - 1] == '\\')) {

				if (key_begin == std::string::npos) {
					key_begin = idx + 1;
					continue;
				}

				auto key_size = idx - key_begin;
				if (key_size == 0) {
					return;
				}

				key = data.substr(key_begin, key_size);
				continue;
			}

			if (key.size() && rune == ':') {
				value_begin = idx + 1;
				break;
			}
		}
		
		if (!key.size() || value_begin == std::string::npos || end - value_begin <= 0) {
			return;
		}

		entries[key] = parse_range(data, value_begin, end);
	};

	for (const auto& index : get_object_entries(data, begin + 1, end - 1)) {
		unwrap(index.first, index.second);
	}

	return Property(entries);
}

Property parse_array(const std::string& data, size_t begin, size_t end) {

	if (data[begin] != '[' || data[end - 1] != ']') {
		throw std::runtime_error("Invalid JSON: Array object: Invalid scope tokens");
	}

	Array entries;
	for (const auto& index : get_object_entries(data, begin + 1, end - 1)) {
		entries.push_back(parse_range(data, index.first, index.second));
	}

	return Property(entries);
}

Property parse_literal(const std::string& data, size_t begin, size_t end) {

	if (end - begin > 4) {
		throw std::runtime_error("Invalid JSON: Parsed string literal too long");
	}

	auto token = data.substr(begin, end - begin);
	if (token == "true") {
		return Property(true);
	} else if (token == "false") {
		return Property(false);
	} else if (token == "null") {
		return Property(nullptr);
	}

	throw std::runtime_error("Invalid JSON: Parsed string literal unknown");
}
