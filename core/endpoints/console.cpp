
#include "../polyfill/polyfill.hpp"
#include "./handlers.hpp"

using namespace Lambda;
using namespace Lambda::Endpoints;

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

std::string Console::serializeEntries(const std::initializer_list<Entry>& list) const {

	std::string temp = Date().toHRTString() + " [" + this->id + "]";

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp += '\n';

	return temp;
}

void Console::log(std::initializer_list<Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stdout);
}

void Console::error(std::initializer_list<Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

void Console::warn(std::initializer_list<Entry> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}
