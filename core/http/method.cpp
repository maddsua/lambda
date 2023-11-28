#include "../core.hpp"
#include <stdexcept>
#include <set>

using namespace HTTP;
using namespace Strings;

static const std::set<std::string> httpKnownMethods = {
	"GET",
	"POST",
	"PUT",
	"DELETE",
	"HEAD",
	"OPTIONS",
	"TRACE",
	"PATCH",
	"CONNECT"
};
static const auto methodsLength = sizeof(httpKnownMethods) / sizeof(std::string);

void Method::apply(const std::string& method) {

	const auto newMethod = Strings::toUpperCase(Strings::trim(method));

	if (!httpKnownMethods.contains(newMethod)) {
		throw new std::runtime_error("\"" + method + "\" is not a valid http method");
	}

	this->value = newMethod;
}
