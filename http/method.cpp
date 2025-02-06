#include "./http.hpp"
#include "./http_utils.hpp"

using namespace Lambda;

const std::string method_get_string = "GET";
const std::string method_post_string = "POST";
const std::string method_put_string = "PUT";
const std::string method_patch_string = "PATCH";
const std::string method_del_string = "DELETE";
const std::string method_head_string = "HEAD";
const std::string method_options_string = "OPTIONS";
const std::string method_trace_string = "TRACE";
const std::string method_connect_string = "CONNECT";

const std::map<std::string, Method> table_method = {
	{ method_get_string, Method::GET },
	{ method_post_string, Method::POST },
	{ method_put_string, Method::PUT },
	{ method_patch_string, Method::PATCH },
	{ method_del_string, Method::DEL },
	{ method_head_string, Method::HEAD },
	{ method_options_string, Method::OPTIONS },
	{ method_trace_string, Method::TRACE },
	{ method_connect_string, Method::CONNECT },
};

const std::string& HTTP::method_to_string(Method method) {
	switch (method) {
		case Method::GET: return method_get_string;
		case Method::POST: return method_post_string;
		case Method::PUT: return method_put_string;
		case Method::PATCH: return method_patch_string;
		case Method::DEL: return method_del_string;
		case Method::HEAD: return method_head_string;
		case Method::OPTIONS: return method_options_string;
		case Method::TRACE: return method_trace_string;
		case Method::CONNECT: return method_connect_string;
		default:
			throw std::runtime_error("Invalid http method enum value");
	}
}

std::optional<Method> HTTP::string_to_method(const std::string& method) {

	auto entry = table_method.find(method);
	if (entry == table_method.end()) {
		return std::nullopt;
	}

	return entry->second;
}

bool HTTP::method_can_have_body(Method method) {
	return method == Method::POST || method == Method::PUT || method == Method::PATCH || method == Method::CONNECT;
}
