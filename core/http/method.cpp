#include "./http.hpp"
#include "../polyfill/polyfill.hpp"

#include <stdexcept>
#include <map>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Strings;

static const std::map<std::string, Methods> httpKnownMethods = {
	{ "GET", Methods::GET },
	{ "POST", Methods::POST },
	{ "PUT", Methods::PUT },
	{ "DELETE", Methods::DEL },
	{ "HEAD", Methods::HEAD },
	{ "OPTIONS", Methods::OPTIONS },
	{ "TRACE", Methods::TRACE },
	{ "PATCH", Methods::PATCH },
	{ "CONNECT", Methods::CONNECT },
};

Method::Method() {
	this->value = Methods::GET;
}

Method::Method(const std::string& method) {
	this->apply(method);
}

Method::Method(const char* method) {
	this->apply(method);
}

std::string Method::toString() const noexcept {
	for (const auto& entry : httpKnownMethods) {
		if (entry.second == this->value) {
			return entry.first;
		}
	}
	return "GET";
}

Method::operator std::string () const noexcept {
	return this->toString();
}

Method::operator Methods () const noexcept {
	return this->value;
}

void Method::apply(const std::string& method) {

	auto knownMethod = httpKnownMethods.find(Strings::toUpperCase(Strings::trim(method)));

	if (knownMethod == httpKnownMethods.end()) {
		throw std::invalid_argument("provided http method is unknown");
	}

	this->value = knownMethod->second;
}
