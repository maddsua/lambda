#include "./http.hpp"

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

std::string Date::to_utc_string() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &this->m_tms);
	return timebuff;
}

std::string Date::date() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y", &this->m_tms);
	return timebuff;
}

std::string Date::to_log_string() const noexcept {
	char timebuff[64];
	strftime(timebuff, sizeof(timebuff), "%H:%M:%S %d/%m/%Y", &this->m_tms);
	return timebuff;
}
