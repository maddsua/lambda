#include "./polyfill.hpp"

#include <chrono>
#include <ctime>

#ifdef _WIN32
	#define gmtime_r(timettime, tmstruct) gmtime_s(tmstruct, timettime)
#endif

using namespace Lambda;

Date::Date() noexcept {
	this->epoch = std::time(nullptr);
	gmtime_r(&this->epoch, &this->timestruct);
}

Date::Date(time_t epoch) noexcept {
	this->epoch = epoch;
	gmtime_r(&epoch, &this->timestruct);
}

time_t Date::getTime() const noexcept {
	return this->epoch;
}

std::string Date::toUTCString() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &this->timestruct);
	return timebuff;
}

std::string Date::toHRTString() const noexcept {
	char timebuff[32];
	strftime(timebuff, sizeof(timebuff), "%d %b %Y %H:%M:%S", &this->timestruct);
	return std::string(timebuff);
}

std::string Date::getDate() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y", &this->timestruct);
	return timebuff;
}

int Date::getSeconds() const noexcept {
	return this->timestruct.tm_sec;
}

int Date::getMonth() const noexcept {
	return this->timestruct.tm_mon;
}

int Date::getMinutes() const noexcept {
	return this->timestruct.tm_min;
}

int Date::getHours() const noexcept {
	return this->timestruct.tm_hour;
}

int Date::getYear() const noexcept {
	return this->timestruct.tm_year;
}

int Date::getDay() const noexcept {
	return this->timestruct.tm_mday;
}
