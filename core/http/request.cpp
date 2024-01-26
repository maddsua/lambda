#include "./http.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Request::Request(
	const std::string& urlinit
) : url(urlinit) {}

Request::Request(
	const std::string& urlinit,
	const Method& methodInit
) : url(urlinit), method(methodInit) {}

Request::Request(
	const std::string& urlinit,
	const Headers& headersinit
) : url(urlinit), headers(headersinit) {}

Request::Request(
	const std::string& urlinit,
	const Method& methodInit,
	const BodyBuffer& bodyinit
) : url(urlinit), method(methodInit), body(bodyinit) {}

Request::Request(
	const std::string& urlinit,
	const Method& methodInit,
	const Headers& headersinit,
	BodyBuffer& bodyinit
) : url(urlinit), method(methodInit), headers(headersinit), body(bodyinit) {}

URL Request::unwrapURL() const {
	return URL(this->url);
}

Cookies Request::getCookies() const {
	auto cookieHeader = this->headers.get("cookie");
	if (!cookieHeader.size()) return {};
	return Lambda::HTTP::Cookies(cookieHeader);
}
