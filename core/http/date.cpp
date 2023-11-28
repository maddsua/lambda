#include "../core.hpp"
#include <chrono>
#include <ctime>

#ifdef _WIN32
#define gmtime_r(timettime, tmstruct) gmtime_s(tmstruct, timettime)
#endif

using namespace HTTP;
using namespace Strings;

Date::Date() {
	this->epoch = std::time(nullptr);
	gmtime_r(&this->epoch, &this->timestruct);
}

Date::Date(time_t epoch) {
	this->epoch = epoch;
	gmtime_r(&epoch, &this->timestruct);
}

time_t Date::getTime() {
	return this->epoch;
}

std::string Date::toUTCString() {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff) - 1, "%a, %d %b %Y %H:%M:%S GMT", &this->timestruct);
	return timebuff;
}
