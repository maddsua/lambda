#include <ctime>

#include "./websocket.hpp"

using namespace Lambda;
using namespace Lambda::Websocket;

Message::Message(const std::string& dataInit) :
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	binary(false),
	partial(false),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::string& dataInit, bool partialInit) :
	data(std::vector<uint8_t>(dataInit.begin(), dataInit.end())),
	binary(false),
	partial(partialInit),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit) :
	data(dataInit),
	binary(true),
	partial(false),
	timestamp(std::time(nullptr))
{}

Message::Message(const std::vector<uint8_t>& dataInit, bool partialInit) :
	data(dataInit),
	binary(true),
	partial(partialInit),
	timestamp(std::time(nullptr))
{}

std::string Message::text() const noexcept {
	return std::string(this->data.begin(), this->data.end());
}

size_t Message::size() const noexcept {
	return this->data.size();
}
