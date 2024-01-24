
#include "../polyfill/polyfill.hpp"
#include "./handlers.hpp"

using namespace Lambda;
using namespace Lambda::Server::Handlers;

LogItem::LogItem(const std::string& thing) {
	this->value = thing;
}
LogItem::LogItem(const char* thing) {
	this->value = thing;
}
LogItem::LogItem(bool thing) {
	this->value = thing ? "true" : "false";
}
LogItem::LogItem(char thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(unsigned char thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(short thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(unsigned short thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(int thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(unsigned int thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(float thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(double thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(long thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(unsigned long thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(long long thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(unsigned long long thing) {
	this->value = std::to_string(thing);
}
LogItem::LogItem(long double thing) {
	this->value = std::to_string(thing);
}

const std::string& LogItem::unwrap() {
	return this->value;
}

std::string Console::serializeEntries(const std::initializer_list<LogItem>& list) const {

	std::string temp = Date().toHRTString() + " [" + this->id + "]";

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.unwrap();
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
