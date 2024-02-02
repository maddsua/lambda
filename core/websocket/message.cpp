#include <ctime>

#include "./websocket.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

Message::Message(const std::string& dataInit) :
	binary(false),
	partial(false),
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::string& dataInit, bool partialInit) :
	binary(false),
	partial(partialInit),
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit) :
	binary(true),
	partial(false),
	data(dataInit),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit, bool partialInit) :
	binary(true),
	partial(partialInit),
	data(dataInit),
	timestamp(std::time(nullptr))
{}

std::string Message::text() const noexcept {
	return std::string(this->data.begin(), this->data.end());
}

size_t Message::size() const noexcept {
	return this->data.size();
}
