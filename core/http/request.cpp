#include "./http.hpp"

using namespace Lambda::HTTP;

Request::Request(const URL& urlinit) : url(urlinit) {}

Request::Request(
	const URL& urlinit,
	const Headers& headersinit
) : url(urlinit), headers(headersinit) {}

Request::Request(
	const URL& urlinit,
	const Body& bodyinit
) : url(urlinit), body(bodyinit) {}

Request::Request(
	const URL& urlinit,
	const Headers& headersinit,
	Body& bodyinit
) : url(urlinit), headers(headersinit), body(bodyinit) {}
