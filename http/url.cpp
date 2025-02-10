#include "./http.hpp"
#include "./http_utils.hpp"

#include <stdexcept>

using namespace Lambda;

URL::URL(const std::string& url) {

	//	get url schema and shift first pointer
	auto scheme_end = url.find("://");
	if (scheme_end != std::string::npos) {
		this->scheme = HTTP::reset_case(url.substr(0, scheme_end));
		scheme_end += 3;
	} else {
		scheme_end = 0;
	}

	//	get the start of a url path
	auto path_begin = std::string::npos;
	for (size_t idx = scheme_end; idx < url.size(); idx++) {
		if (url[idx] == '/') {
			path_begin = idx;
			break;
		}
	}

	//	path is always required
	if (path_begin == std::string::npos) {
		throw std::runtime_error("URL: Parsing error: Path is not defined");
	}

	auto parse_credentials = [&](size_t token_begin, size_t token_end) -> std::optional<BasicAuth> {

		if (token_end - token_begin < 2) {
			return std::nullopt;
		}

		auto auth_split = std::string::npos;
		for (size_t idx = token_begin; idx < token_end; idx++) {
			if (url[idx] == ':') {
				auth_split = idx;
				break;
			}
		}

		if (auth_split == std::string::npos) {
			return BasicAuth { .user = url.substr(token_begin, token_end - token_begin) };
		}

		if (auth_split - token_begin < 1) {
			return std::nullopt;
		}

		auto password_start = auth_split + 1;

		BasicAuth auth { .user = url.substr(token_begin, auth_split - token_begin) };

		if (token_end - password_start > 0) {
			auth.password = url.substr(password_start, token_end - password_start);
		}
		
		return auth;
	};

	//	get user auth string
	auto auth_end = std::string::npos;
	for (size_t idx = scheme_end; idx < path_begin; idx++) {
		if (url[idx] == '@') {
			auth_end = idx;
			break;
		}
	}

	//	get basic auth and host name
	if (auth_end != std::string::npos) {
		auto path_begin = auth_end + 1;
		this->host = HTTP::reset_case(url.substr(path_begin, path_begin - path_begin));
		this->user = parse_credentials(scheme_end, auth_end);
	} else {
		this->host = HTTP::reset_case(url.substr(scheme_end, path_begin - scheme_end));
	}

	//	mark fragment
	auto fragment_begin = url.size();
	for (size_t idx = path_begin; idx < url.size(); idx++) {
		if (url[idx] == '#') {
			this->fragment = url.substr(idx, url.size() - idx);
			fragment_begin = idx;
			break;
		}
	}

	//	get query
	auto query_begin = fragment_begin;
	for (size_t idx = path_begin; idx < fragment_begin; idx++) {
		if (url[idx] == '?') {
			auto search_start = idx + 1;
			this->search = URLSearchParams(url.substr(search_start, fragment_begin - search_start));
			query_begin = idx;
			break;
		}
	}

	this->path = url.substr(path_begin, query_begin - path_begin);
}

std::string URL::to_string() const noexcept {

	std::string url;

	if (!this->host.empty()) {

		if (!this->scheme.empty()) {
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

	if (!this->path.empty()) {
		url.append(this->path);
	}

	if (!this->search.empty()) {
		url.push_back('?');
		url.append(this->search.to_string());
	}

	if (!this->fragment.empty()) {

		if (!this->fragment.starts_with('#')) {
			url.push_back('#');
		}

		url.append(this->fragment);
	}

	return url;
}

std::string URL::href() const {

	std::string url;

	if (this->scheme.empty()) {
		throw std::runtime_error("URL: Cannot create href: Scheme missing");
	}

	url.append(this->scheme);

	if (this->host.empty()) {
		throw std::runtime_error("URL: Cannot create href from a relative URL");
	}

	url.append("://");
	url.append(this->host);

	if (this->path.empty()) {
		throw std::runtime_error("URL: Cannot create href: Path missing");
	}

	url.append(this->path);

	if (!this->search.empty()) {
		url.push_back('?');
		url.append(this->search.to_string());
	}

	return url;
}
