#include "./http.hpp"

using namespace Lambda::HTTP;

Response::Response(const Status& statusinit) : status(statusinit) {}

Response::Response(const Headers& headersinit) : headers(headersinit) {}

Response::Response(const Body& bodyinit) : body(bodyinit) {}

Response::Response(
	const Headers& headersinit,
	const Body& bodyinit
) : headers(headersinit), body(bodyinit) {}

Response::Response(
	const Status& statusinit,
	const Headers& headersinit
) : status(statusinit), headers(headersinit) {}

Response::Response(
	const Status& statusinit,
	const Body& bodyinit
) : status(statusinit), body(bodyinit) {}

Response::Response(
	const Status& statusinit,
	const Headers& headersinit,
	const Body& body
) : status(statusinit), headers(headersinit), body(body) {}

Response::Response(
	const Body& body,
	const Headers& headersinit,
	const Status& statusinit
) : status(statusinit), headers(headersinit), body(body) {}
