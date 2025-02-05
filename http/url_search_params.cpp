#include "./http.hpp"

#include <algorithm>

using namespace Lambda;

URLSearchParams::URLSearchParams(const std::string& params) {

	size_t pair_start = 0;
	const auto last_pair_idx = params.size() - 1;

	auto parse_token = [&](size_t token_start, size_t token_end) {

		size_t token_split = token_end;
		const auto last_token_idx = token_end - 1;

		for (size_t idx = token_start; idx < token_end; idx++) {
			if (params[idx] == '=') {
				token_split = idx;
				break;
			}
		}

		if (token_split == token_start || token_split >= last_token_idx) {
			return;
		}

		const auto key = params.substr(token_start, token_split - token_start);
		const auto value = params.substr(token_split + 1, token_end - token_split - 1);

		this->append(decode_uri_component(key), decode_uri_component(value));
	};

	for (size_t idx = pair_start; idx < params.size(); idx++) {
		if (params[idx] == '&') {
			parse_token(pair_start, idx);
			pair_start = idx + 1;
		} else if (idx == last_pair_idx) {
			parse_token(pair_start, params.size());
			break;
		}
	}
}

std::string URLSearchParams::to_string() const noexcept {

	std::string search;

	for (const auto& entry : this->m_entries) {
		for (const auto& value : entry.second) {

			if (search.size()) {
				search.push_back('&');
			}

			search.append(encode_uri_component(entry.first));
			search.push_back('=');
			search.append(encode_uri_component(value));
		}
	}

	return search;
}
