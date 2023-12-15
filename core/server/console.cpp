
#include "./server.hpp"
#include "../polyfill/polyfill.hpp"

using namespace Lambda;


Server::LogItem::LogItem(const std::string& thing) {
	this->value = thing;
}
Server::LogItem::LogItem(const char* thing) {
	this->value = thing;
}
Server::LogItem::LogItem(bool thing) {
	this->value = thing ? "true" : "false";
}
Server::LogItem::LogItem(char thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(unsigned char thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(short thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(unsigned short thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(int thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(unsigned int thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(float thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(double thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(long thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(unsigned long thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(long long thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(unsigned long long thing) {
	this->value = std::to_string(thing);
}
Server::LogItem::LogItem(long double thing) {
	this->value = std::to_string(thing);
}

std::string Server::Console::serializeEntries(const std::initializer_list<LogItem>& list) const {

	std::string temp = Date().toHRTString() + " [" + this->id + "]";

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp += '\n';

	return temp;
}

void Server::Console::log(std::initializer_list<LogItem> list ) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

void Server::Console::error(std::initializer_list<LogItem> list ) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

void Server::Console::warn(std::initializer_list<LogItem> list ) const {
	std::string temp = this->serializeEntries(list);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}
