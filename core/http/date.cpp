#include "../core.hpp"
#include <chrono>
#include <ctime>

using namespace HTTP;
using namespace Strings;

Date::Date() {
	this->epoch = std::time(nullptr);
}

Date::Date(time_t epoch) {
	this->epoch = epoch;
}
time_t Date::getTime() {
	return this->epoch;
}
