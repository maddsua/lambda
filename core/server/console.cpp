
#include "../polyfill/polyfill.hpp"
#include "./console.hpp"

using namespace Lambda;

Console::Entry::Entry(const std::string& thing) {
	this->value = thing;
}
Console::Entry::Entry(const char* thing) {
	this->value = thing;
}
Console::Entry::Entry(bool thing) {
	this->value = thing ? "true" : "false";
}
Console::Entry::Entry(char thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(unsigned char thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(short thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(unsigned short thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(int thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(unsigned int thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(float thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(double thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(long thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(unsigned long thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(long long thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(unsigned long long thing) {
	this->value = std::to_string(thing);
}
Console::Entry::Entry(long double thing) {
	this->value = std::to_string(thing);
}

Console::Console(
	const std::string& setid
) : m_id(setid) {}

Console::Console(
	const std::string& setid,
	bool useTimestamps
) : m_id(setid), m_timestamps(useTimestamps) {}

std::string Console::serializeEntries(const std::initializer_list<Console::Entry>& list) const {

	std::string temp;

	if (this->m_timestamps) {
		temp.append(Date().toHRTString());
		temp.push_back(' ');
	}

	if (this->m_id.size()) {
		temp.push_back('[');
		temp.append(this->m_id);
		temp.append("] ");
	}

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp.push_back('\n');

	return temp;
}

void Console::log(std::initializer_list<Console::Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stdout);
}

void Console::error(std::initializer_list<Console::Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

void Console::warn(std::initializer_list<Console::Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}
