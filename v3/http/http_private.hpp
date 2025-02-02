#ifndef __LAMBDA_HTTP_PRIVATE__
#define __LAMBDA_HTTP_PRIVATE__

#include <string>

#include "./http.hpp"
#include "./http_utils.hpp"

namespace Lambda::HTTP {

	CookieValues parse_cookie(const std::string& header);

	std::optional<BasicAuth> parse_basic_auth(const std::string& header);
};

#endif
