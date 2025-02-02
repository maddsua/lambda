#include "./polyfill.hpp"

#include <chrono>
#include <ctime>

#ifdef _WIN32
	#define gmtime_r(timettime, tmstruct) gmtime_s(tmstruct, timettime)
#endif

using namespace Lambda;

Date::Date() noexcept {
	this->m_unix = std::time(nullptr);
	gmtime_r(&this->m_unix, &this->m_tms);
}

Date::Date(time_t epoch) noexcept {
	this->m_unix = epoch;
	gmtime_r(&epoch, &this->m_tms);
}

time_t Date::epoch() const noexcept {
	return this->m_unix;
}

std::string Date::to_utc_string() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &this->m_tms);
	return timebuff;
}

std::string Date::to_calendar_string() const noexcept {
	char timebuff[32];
	strftime(timebuff, sizeof(timebuff), "%d %b %Y %H:%M:%S", &this->m_tms);
	return std::string(timebuff);
}

std::string Date::date() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y", &this->m_tms);
	return timebuff;
}

int Date::second() const noexcept {
	return this->m_tms.tm_sec;
}

int Date::month() const noexcept {
	return this->m_tms.tm_mon;
}

int Date::minute() const noexcept {
	return this->m_tms.tm_min;
}

int Date::hour() const noexcept {
	return this->m_tms.tm_hour;
}

int Date::year() const noexcept {
	return this->m_tms.tm_year;
}

int Date::get_date() const noexcept {
	return this->m_tms.tm_mday;
}
