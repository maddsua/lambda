#include "../http.hpp"
#include "../polyfill.hpp"

#include <chrono>
#include <ctime>

#ifdef _WIN32
#define gmtime_r(timettime, tmstruct) gmtime_s(tmstruct, timettime)
#endif

using namespace Lambda;

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
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &this->timestruct);
	return timebuff;
}

std::string Date::toHRTString() {
	char timebuff[32];
	strftime(timebuff, sizeof(timebuff), "%d %b %Y %H:%M:%S", &this->timestruct);
	return std::string(timebuff);
}

std::string Date::getDate() {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y", &this->timestruct);
	return timebuff;
}

int Date::getSeconds() {
	return this->timestruct.tm_sec;
}

int Date::getMonth() {
	return this->timestruct.tm_mon;
}

int Date::getMinutes() {
	return this->timestruct.tm_min;
}

int Date::getHours() {
	return this->timestruct.tm_hour;
}

int Date::getYear() {
	return this->timestruct.tm_year;
}

int Date::getDay() {
	return this->timestruct.tm_mday;
}
