
#include "../polyfill/polyfill.hpp"
#include "./utils.hpp"

using namespace Lambda;
using namespace Lambda::SyncOut;

MgsOverload::MgsOverload(const std::string& thing) {
	if (!thing.size()) return;
	this->valueOpt = thing;
}
MgsOverload::MgsOverload(const char* thing) {
	if (thing == nullptr || !thing[0]) return;
	this->valueOpt = thing;
}
MgsOverload::MgsOverload(bool thing) {
	this->valueOpt = thing ? "true" : "false";
}
MgsOverload::MgsOverload(char thing) {
	this->valueOpt = std::string(1, thing);
}
MgsOverload::MgsOverload(unsigned char thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(short thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned short thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(int thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned int thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(float thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(double thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(long thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned long thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(long long thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(unsigned long long thing) {
	this->valueOpt = std::to_string(thing);
}
MgsOverload::MgsOverload(long double thing) {
	this->valueOpt = std::to_string(thing);
}

std::optional<std::string> WrapperImpl::serializeEntries(const std::initializer_list<MgsOverload>& list) const noexcept {

	std::string temp;

	for (auto elem : list) {
		if (!elem.valueOpt.has_value()) continue;
		if (temp.size()) temp.push_back(' ');
		temp += elem.valueOpt.value();
	}

	if (!temp.size()) return std::nullopt;

	temp.push_back('\n');

	return temp;
}

void WrapperImpl::log(std::initializer_list<MgsOverload> list) noexcept {

	auto temp = this->serializeEntries(list);
	if (!temp.has_value()) return;

	auto& value = temp.value();
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(value.c_str(), sizeof(char), value.size(), stdout);
}

void WrapperImpl::error(std::initializer_list<MgsOverload> list) noexcept {

	auto temp = this->serializeEntries(list);
	if (!temp.has_value()) return;

	auto& content = temp.value();
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(content.c_str(), sizeof(char), content.size(), stderr);
}

void WrapperImpl::log(MgsOverload item) noexcept {

	if (!item.valueOpt.has_value()) return;

	auto content = item.valueOpt.value() + '\n';
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(content.c_str(), sizeof(char), content.size(), stdout);
}

void WrapperImpl::error(MgsOverload item) noexcept {

	if (!item.valueOpt.has_value()) return;

	auto content = item.valueOpt.value() + '\n';
	std::lock_guard<std::mutex> lock(this->m_write_lock);
	fwrite(content.c_str(), sizeof(char), content.size(), stderr);
}

SyncOut::WrapperImpl Lambda::syncout = WrapperImpl();
