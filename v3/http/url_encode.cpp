#include "./http.hpp"

bool rune_url_safe(const char rune) noexcept {

	if (rune >= '0' && rune <= '9') {
		return true;
	} else if (rune >= 'A' && rune <= 'Z') {
		return true;
	} else if (rune >= 'a' && rune <= 'z') {
		return true;
	}

	static const char safe_list[] = "$-_.+!*'()";
	for (size_t idx = 0; idx < sizeof(safe_list); idx++) {
		if (rune == safe_list[idx]) {
			return true;
		}
	}

	return false;
}

static const char hex_encode_table[] = {
	'0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F'
};

std::string Lambda::encode_uri_component(const std::string& input) {

	std::string result;
	result.reserve(input.size());

	for (const auto rune : input) {

		if (rune_url_safe(rune)) {
			result.push_back(rune);
			continue;
		}

		result.push_back('%');
		result.push_back(hex_encode_table[(rune & 0xF0) >> 4]);
		result.push_back(hex_encode_table[rune & 0x0F]);
	}

	return result;
}

std::string Lambda::decode_uri_component(const std::string& input) {

	std::string result;
	result.reserve(input.size());

	for (size_t i = 0; i < input.size(); i++) {

		auto rune = input[i];

		if (rune == '%') {

			auto decoded_rune = std::stoi(input.substr(i + 1, 2), nullptr, 16);
			result.push_back(decoded_rune);

			i += 2;
			continue;
		}

		result.push_back(rune);
	}

	return result;
}
