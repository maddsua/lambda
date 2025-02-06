#include "./http.hpp"
#include "./http_utils.hpp"

using namespace Lambda;

CookieValues HTTP::parse_cookie(const std::string& header) {

	HTTP::Values values;

	auto parse_pair = [&](size_t begin, size_t end) {

		auto split_token = std::string::npos;
		for (size_t idx = begin; idx < end; idx++) {
			if (header[idx] == '=') {
				split_token = idx;
				break;
			}
		}

		if (split_token == std::string::npos) {
			return;
		}

		auto value_begin = split_token + 1;
		if (end - value_begin < 1) {
			return;
		}

		values.append(header.substr(begin, split_token - begin), header.substr(value_begin, end - value_begin));
	};

	size_t pair_begin = 0;
	auto seek_end = header.size() - 1; 

	for (size_t idx = 1; idx < header.size(); idx++) {
		
		//	get last value
		if (idx == seek_end) {
			parse_pair(pair_begin, header.size());
			break;
		}

		//	split pairs
		if (header[idx - 1] == ';' && header[idx] == ' ') {
			parse_pair(pair_begin, idx - 1);
			pair_begin = idx + 1;
		}
	}

	return values;
}

std::string Cookie::to_string() const {

	if (this->name.contains(';')) {
		throw std::runtime_error("Invalid cookie name: Should not contain semicolons");
	}

	if (this->value.contains(';')) {
		throw std::runtime_error("Invalid cookie value: Should not contain semicolons");
	}

	std::string set_cookie;

	set_cookie.append(this->name);
	set_cookie.push_back('=');
	set_cookie.append(this->value);

	auto write_boolean_prop = [&](const char* prop) {
		set_cookie.append("; ");
		set_cookie.append(prop);
	};

	auto write_prop = [&](const char* prop, const std::string& value) {
		write_boolean_prop(prop);
		set_cookie.push_back('=');
		set_cookie.append(value);
	};

	if (!this->domain.empty()) {
		write_prop("Domain", this->domain);
	}

	if (!this->path.empty()) {
		write_prop("Path", this->path);
	}

	if (this->expires.has_value()) {
		write_prop("Expires", Date(this->expires.value()).to_utc_string());
	}

	if (this->max_age.has_value()) {
		write_prop("Max-Age", Date(this->max_age.value()).to_utc_string());
	}

	if (this->secure) {
		write_boolean_prop("Secure");
	}

	if (this->http_only) {
		write_boolean_prop("HttpOnly");
	}

	switch (this->same_site) {
		case SameSite::None: {
			write_prop("SameSite", "None");
		} break;
		case SameSite::Lax: {
			write_prop("SameSite", "Lax");
		} break;
		case SameSite::Strict: {
			write_prop("SameSite", "Strict");
		} break;
		default: break;
	}

	return set_cookie;
}
