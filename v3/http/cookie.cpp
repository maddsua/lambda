#include "./http.hpp"

#include <stdexcept>

using namespace Lambda;

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

	if (this->domain.size()) {
		write_prop("Domain", this->domain);
	}

	if (this->path.size()) {
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
