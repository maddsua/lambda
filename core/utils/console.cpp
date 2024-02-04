
#include "../polyfill/polyfill.hpp"
#include "./utils.hpp"

using namespace Lambda;
using namespace Lambda::Console;

LogEntry::LogEntry(const std::string& thing) {
	this->value = thing;
}
LogEntry::LogEntry(const char* thing) {
	this->value = thing;
}
LogEntry::LogEntry(bool thing) {
	this->value = thing ? "true" : "false";
}
LogEntry::LogEntry(char thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(unsigned char thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(short thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(unsigned short thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(int thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(unsigned int thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(float thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(double thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(long thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(unsigned long thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(long long thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(unsigned long long thing) {
	this->value = std::to_string(thing);
}
LogEntry::LogEntry(long double thing) {
	this->value = std::to_string(thing);
}

std::string GlobalSyncConsole::serializeEntries(const std::initializer_list<LogEntry>& list) const noexcept {

	std::string temp;

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp.push_back('\n');

	return temp;
}

void GlobalSyncConsole::log(std::initializer_list<LogEntry> list) noexcept {
	std::string temp = this->serializeEntries(list);
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stdout);
}

void GlobalSyncConsole::error(std::initializer_list<LogEntry> list) noexcept {
	std::string temp = this->serializeEntries(list);
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

Console::GlobalSyncConsole Lambda::console = GlobalSyncConsole();
