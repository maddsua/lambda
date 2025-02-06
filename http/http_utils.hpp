#ifndef __LIB_MADDSUA_LAMBDA_http_utils__
#define __LIB_MADDSUA_LAMBDA_http_utils__

#include <string>

#include "./http.hpp"

namespace Lambda::HTTP {

	CookieValues parse_cookie(const std::string& header);

	std::optional<BasicAuth> parse_basic_auth(const std::string& header);

	const std::string& method_to_string(Method method);
	std::optional<Method> string_to_method(const std::string& method);
	bool method_can_have_body(Method method);

	std::string reset_case(std::string value);
};

#endif
