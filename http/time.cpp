#include "./http.hpp"

#include <string.h>
#include <time.h>

std::string Lambda::HTTP::serverDate(time_t epoch_time) {
	char timebuff[128];
	tm tms = *gmtime(&epoch_time);
	strftime(timebuff, sizeof(timebuff), "%a, %d %b %Y %H:%M:%S GMT", &tms);
	return timebuff;
}
std::string Lambda::HTTP::serverDate() {
	return serverDate(time(nullptr));
}

std::string Lambda::HTTP::serverTime(time_t timestamp) {
	char timebuff[32];
	auto timedata = gmtime(&timestamp);
	strftime(timebuff, sizeof(timebuff), "%H:%M:%S", timedata);
	return std::string(timebuff);
}
std::string Lambda::HTTP::serverTime() {
	return serverTime(time(nullptr));
}
