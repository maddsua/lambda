#include "../http.hpp"
#include "../polyfill.hpp"

#include <stdexcept>
#include <set>

using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::Strings;

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
		throw std::invalid_argument("provided http method is unknown");
	}

	this->value = newMethod;
}
