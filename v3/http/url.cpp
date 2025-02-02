#include "./http.hpp"

#include <stdexcept>

using namespace Lambda;

//	todo: do component validation
URL::URL(const std::string& url) {

	//	get url schema and shift first pointer
	auto scheme_token = url.find("://");
	if (scheme_token != std::string::npos) {
		this->scheme = url.substr(0, scheme_token);
		scheme_token += 3;
	} else {
		scheme_token = 0;
	}

	//	get the start of a url path
	auto path_token = std::string::npos;
	for (size_t idx = scheme_token; idx < url.size(); idx++) {
		if (url[idx] == '/') {
			path_token = idx;
			break;
		}
	}

	//	path is always required
	if (path_token == std::string::npos) {
		throw std::runtime_error("URL: Parsing error: Path is not defined");
	}

	auto parse_credentials = [&](size_t token_start, size_t token_end) -> std::optional<BasicAuth> {

		if (token_end - token_start < 2) {
			return std::nullopt;
		}

		auto auth_split = std::string::npos;
		for (size_t idx = token_start; idx < token_end; idx++) {
			if (url[idx] == ':') {
				auth_split = idx;
				break;
			}
		}

		if (auth_split == std::string::npos) {
			return BasicAuth { .user = url.substr(token_start, token_end - token_start) };
		}

		if (auth_split - token_start < 1) {
			return std::nullopt;
		}

		auto password_start = auth_split + 1;

		BasicAuth auth { .user = url.substr(token_start, auth_split - token_start) };

		if (token_end - password_start > 0) {
			auth.password = url.substr(password_start, token_end - password_start);
		}
		
		return auth;
	};

	//	get user auth string
	auto auth_token = std::string::npos;
	for (size_t idx = scheme_token; idx < path_token; idx++) {
		if (url[idx] == '@') {
			auth_token = idx;
			break;
		}
	}

	//	get basic auth and host name
	if (auth_token != std::string::npos) {
		auto path_start = auth_token + 1;
		this->host = url.substr(path_start, path_token - path_start);
		this->user = parse_credentials(scheme_token, auth_token);
	} else {
		this->host = url.substr(scheme_token, path_token - scheme_token);
	}

	//	mark fragment
	auto fragment_token = url.size();
	for (size_t idx = path_token; idx < url.size(); idx++) {
		if (url[idx] == '#') {
			this->fragment = url.substr(idx, url.size() - idx);
			fragment_token = idx;
			break;
		}
	}

	//	get query
	auto query_token = fragment_token;
	for (size_t idx = path_token; idx < fragment_token; idx++) {
		if (url[idx] == '?') {
			auto search_start = idx + 1;
			this->search = URLSearchParams(url.substr(search_start, fragment_token - search_start));
			query_token = idx;
			break;
		}
	}

	this->path = url.substr(path_token, query_token - path_token);
}

std::string URL::to_string() const noexcept {

	std::string url;

	if (this->host.size()) {

		if (this->scheme.size()) {
			url.append(this->scheme);
		}

		url.append("://");

		if (this->user.has_value()) {
			auto& user = this->user.value();
			if (user.user.size() && user.password.size()) {
				url.append(encode_uri_component(user.user));
				url.push_back(':');
				url.append(encode_uri_component(user.password));
				url.push_back('@');
			}
		}

		url.append(this->host);
	}

	if (this->path.size()) {
		url.append(this->path);
	}

	if (this->search.size()) {
		url.push_back('?');
		url.append(this->search.to_string());
	}

	return url;
}

std::string URL::href() const {

	if (!this->scheme.size()) {
		throw std::runtime_error("URL: Cannot create href: Scheme missing");
	}

	if (!this->host.size()) {
		throw std::runtime_error("URL: Cannot create href from a relative URL");
	}

	if (!this->path.size()) {
		throw std::runtime_error("URL: Cannot create href: Path missing");
	}

	return this->to_string();
}
