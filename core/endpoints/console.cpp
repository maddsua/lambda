
#include "../polyfill/polyfill.hpp"
#include "./handlers.hpp"

using namespace Lambda;
using namespace Lambda::Endpoints;

Console::LogItem::LogItem(const std::string& thing) {
	this->value = thing;
}
Console::LogItem::LogItem(const char* thing) {
	this->value = thing;
}
Console::LogItem::LogItem(bool thing) {
	this->value = thing ? "true" : "false";
}
Console::LogItem::LogItem(char thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(unsigned char thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(short thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(unsigned short thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(int thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(unsigned int thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(float thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(double thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(long thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(unsigned long thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(long long thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(unsigned long long thing) {
	this->value = std::to_string(thing);
}
Console::LogItem::LogItem(long double thing) {
	this->value = std::to_string(thing);
}

std::string Console::serializeEntries(const std::initializer_list<LogItem>& list) const {

	std::string temp = Date().toHRTString() + " [" + this->id + "]";

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp += '\n';

	return temp;
}

void Console::log(std::initializer_list<LogItem> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stdout);
}

void Console::error(std::initializer_list<LogItem> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

void Console::warn(std::initializer_list<LogItem> list) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}
