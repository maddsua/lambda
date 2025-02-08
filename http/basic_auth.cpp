#include "./http.hpp"
#include "./http_private.hpp"
#include "../base64/base64.hpp"

using namespace Lambda;

std::optional<BasicAuth> HTTP::parse_basic_auth(const std::string& header) {

	if (header.empty()) {
		return std::nullopt;
	}

	auto scheme_end = std::string::npos;
	auto token_begin = std::string::npos;

	for (size_t idx = 0; idx < header.size(); idx++) {

		auto rune_is_space = header[idx] == ' ';

		if (rune_is_space && scheme_end == std::string::npos) {
			scheme_end = idx;
		} else if (!rune_is_space && scheme_end != std::string::npos) {
			token_begin = idx;
			break;
		}
	}

	if (scheme_end == std::string::npos || token_begin == std::string::npos) {
		return std::nullopt;
	}

	auto scheme = header.substr(0, scheme_end);
	for (auto& rune : scheme) {
		if (rune >= 'A' && rune <= 'Z') {
			rune += 0x20;
		}
	}

	if (scheme != "basic") {
		return std::nullopt;
	}

	auto token = header.substr(token_begin);
	if (!Encoding::Base64::valid(token)) {
		return std::nullopt;
	}

	auto basic_auth_pair = Encoding::Base64::decode(token);
	auto basic_auth_split = std::string::npos;

	for (size_t idx = 0; idx < basic_auth_pair.size(); idx++) {
		if (basic_auth_pair[idx] == ':') {
			basic_auth_split = idx;
			break;
		}
	}

	if (basic_auth_split == std::string::npos || basic_auth_split == 0) {
		return std::nullopt;
	}
	
	BasicAuth auth {
		.user = std::string(basic_auth_pair.begin(), basic_auth_pair.begin() + basic_auth_split),
		.password = std::string(basic_auth_pair.begin() + basic_auth_split + 1, basic_auth_pair.end()),
	};
	
	return auth;
}
