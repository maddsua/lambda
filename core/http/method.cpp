#include "../core.hpp"
#include <stdexcept>

using namespace HTTP;
using namespace Strings;

static const std::string httpKnownMethods[] = {
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

	for (size_t i = 0; i < methodsLength; i++) {
		if (newMethod == httpKnownMethods[i]) {
			this->value = newMethod;
			return;
		}
	}
	
	throw new std::runtime_error("\"" + method + "\" is not a valid http method");
}
