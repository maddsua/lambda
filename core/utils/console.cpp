
#include "../polyfill/polyfill.hpp"
#include "./utils.hpp"

using namespace Lambda;
using namespace Lambda::SyncOut;

MgsOverload::MgsOverload(const std::string& thing) {
	this->value = thing;
}
MgsOverload::MgsOverload(const char* thing) {
	this->value = thing;
}
MgsOverload::MgsOverload(bool thing) {
	this->value = thing ? "true" : "false";
}
MgsOverload::MgsOverload(char thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned char thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(short thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned short thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(int thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned int thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(float thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(double thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(long thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned long thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(long long thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned long long thing) {
	this->value = std::to_string(thing);
}
MgsOverload::MgsOverload(long double thing) {
	this->value = std::to_string(thing);
}

std::string WrapperImpl::serializeEntries(const std::initializer_list<MgsOverload>& list) const noexcept {

	std::string temp;

	for (auto elem : list) {
		if (temp.size()) temp.push_back(' ');
		temp += elem.value;
	}

	temp.push_back('\n');

	return temp;
}

void WrapperImpl::log(std::initializer_list<MgsOverload> list) noexcept {
	std::string temp = this->serializeEntries(list);
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stdout);
}

void WrapperImpl::error(std::initializer_list<MgsOverload> list) noexcept {
	std::string temp = this->serializeEntries(list);
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(temp.c_str(), sizeof(char), temp.size(), stderr);
}

SyncOut::WrapperImpl Lambda::syncout = WrapperImpl();
