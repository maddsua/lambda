#include "./http.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;

Response::Response(
	const Status& statusinit
) : status(statusinit) {}

Response::Response(
	const Status& statusinit,
	const Headers& headersinit
) : status(statusinit), headers(headersinit) {}

Response::Response(
	const Status& statusinit,
	const BodyBuffer& body
) : status(statusinit), body(body) {}

Response::Response(
	const BodyBuffer& bodyinit
) : body(bodyinit) {}

Response::Response(
	const Status& statusinit,
	const Headers& headersinit,
	const BodyBuffer& body
) : status(statusinit), headers(headersinit), body(body) {}

void Response::setCookies(const Cookies& cookies) {
	auto entries = cookies.serialize();
	for (const auto& entry : entries)
		this->headers.append("Set-Cookie", entry);
}
