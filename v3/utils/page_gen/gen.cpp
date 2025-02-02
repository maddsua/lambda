#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <vector>

std::string read_template_stream(std::istream& stream) {

	std::string content;

	auto last_rune = [&](int offset) -> char {

		if (offset >= 0) {
			return '\0';
		};

		if (offset * -1 >= content.size()) {
			return '\0';
		}

		return content.at(content.size() + offset);
	};

	auto is_space = [](char rune) {
		return rune == '\r' || rune == '\n' || rune == '\t'	|| rune == ' ';
	};

	while (!stream.eof()) {

		char rune;
		stream.read(&rune, sizeof(rune));
		if (!stream.gcount()) {

			//	rm trailing space
			if (is_space(last_rune(-1))) {
				content.pop_back();
			}

			break;
		}

		//	trim leading spaces
		if (!content.size() && is_space(rune)) {
			continue;
		}

		//	skip repeating spaces
		if (is_space(rune)) {

			if (is_space(last_rune(-1))) {
				continue;
			}

			rune = ' ';
		}

		//	skip spaces between html tags
		if (rune == '<' && is_space(last_rune(-1)) && last_rune(-2) == '>') {
			content.pop_back();
		}

		content.push_back(rune);
	}

	if (!content.size()) {
		throw std::runtime_error("Empty template content");
	}
	
	return content;
}

struct PageTemplte {
	std::vector<std::string> names;
	std::vector<std::string> chunks;
};

PageTemplte parse_template(const std::string& template_content) {

	auto variable_name = [](const std::string& slice) -> std::optional<std::string> {

		auto is_var_rune = [](const char rune) -> bool {
			return (
				(rune == '_') ||
				(rune >= '0' && rune <= '9') ||
				(rune >= 'a' && rune <= 'z') ||
				(rune >= 'A' && rune <= 'Z') ||
				(rune == ' ' || rune == '\t')
			);
		};

		auto is_space = [](const char rune) -> bool {
			return (rune == ' ' || rune == '\t');
		};

		size_t slice_begin = std::string::npos;
		size_t slice_end = slice.size();

		for (size_t idx = 0; idx < slice.size(); idx++) {

			auto rune = slice[idx];
			if (!is_var_rune(rune)) {
				return std::nullopt;
			}

			if (slice_begin == std::string::npos && !is_space(rune)) {
				slice_begin = idx;
				continue;
			}

			if (slice_begin != std::string::npos && is_space(rune)) {
				slice_end = idx;
				break;
			}
		}

		return slice.substr(slice_begin, slice_end - slice_begin);
	};

	PageTemplte result;

	size_t chunk_begin = 0;

	for (size_t idx = 0; idx < template_content.size(); idx++) {

		auto next_var = template_content.find("{{", idx);
		if (next_var == std::string::npos) {
			result.chunks.push_back(template_content.substr(idx));
			break;
		}

		auto var_begin = next_var + 2;
		auto var_end = template_content.find("}}", next_var);
		if (var_end == std::string::npos) {
			idx = next_var + 1;
			continue;
		}

		idx = var_end + 1;

		auto variable = variable_name(template_content.substr(var_begin, var_end - var_begin));
		if (!variable.has_value()) {
			continue;
		}

		result.names.push_back(variable.value());
		result.chunks.push_back(template_content.substr(chunk_begin, next_var - chunk_begin));
		chunk_begin = var_end + 2;
	}

	return result;
}

std::string escape_chunk (const std::string& chunk) {

	std::string result;
	
	for (const auto rune : chunk) {
		if (rune == '\\' || rune == '"') {
			result.push_back('\\');
		}
		result.push_back(rune);
	}

	return '"' + result + '"';
}

std::string generate_source(const PageTemplte& page_tempate) {

	std::string function_args;
	for (const auto item : page_tempate.names) {
		
		if (function_args.size()) {
			function_args.append(", ");	
		}

		function_args.append("const std::string& ");
		function_args.append(item);
	}
	
	std::string template_body;
	for (size_t idx = 0; idx < page_tempate.chunks.size(); idx++) {

		auto name_idx = idx - 1;

		if (name_idx >= 0 && name_idx < page_tempate.names.size()) {
			template_body.append(" + ");
			template_body.append(page_tempate.names[name_idx]);	
			template_body.append(" + ");
		}
		
		template_body.append(escape_chunk(page_tempate.chunks[idx]));
	}

	std::string result;
	result.append("//\tGenerated function renderer:\n");
	result.append("\n#include <string>\n\n");
	result.append("std::string render(" + function_args + ") {\n");
	result.append("\treturn (\n\t\t" + template_body + "\n\t);\n}");

	return result;
}

int main() {

	//	todo: fix pipe detection
	//	std::cerr << "gen: Welp you gotta launch this program in a pipe" << std::endl;

	if (std::cin.peek() == std::istream::traits_type::eof()) {
		std::cerr << "gen: Piped inputs are empty" << std::endl;
		return 1;
	}

	try {
		auto page_template = parse_template(read_template_stream(std::cin));
		std::cout << generate_source(page_template) << std::endl;
	} catch(const std::exception& e) {
		std::cerr << "gen: Failed to generate template: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
