#include "./http.hpp"
#include "../polyfill/polyfill.hpp"
#include "../encoding/encoding.hpp"

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Auth;

std::optional<BasicCredentials> Auth::parseBasicAuth(const std::string& header) {

	if (!Strings::includes(Strings::toLowerCase(header), "basic")) {
		return std::nullopt;
	}

	const auto authStringStart = header.find_last_of(' ');
	if (authStringStart == std::string::npos) {
		return std::nullopt;
	}

	const auto authString = header.substr(authStringStart + 1);
	if (!authString.size()) {
		return std::nullopt;
	}

	const auto authStringDecoded = Encoding::fromBase64(authString);
	const auto authComponents = Strings::split(std::string(authStringDecoded.begin(), authStringDecoded.end()), ":");
	if (authComponents.size() < 2) {
		return std::nullopt;
	}

	return BasicCredentials {
		authComponents[0],
		authComponents[1]
	};
}
