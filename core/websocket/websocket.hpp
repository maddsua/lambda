#ifndef __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__
#define __LIB_MADDSUA_LAMBDA_CORE_WEBSOCKETS__

#include <string>
#include <vector>
#include <queue>
#include <cstdint>

namespace Lambda::Websocket {

	struct Message {
		const std::vector<uint8_t> data;
		const bool binary = false;
		const bool partial = false;
		const time_t timestamp = 0;

		Message(const std::string& dataInit);
		Message(const std::string& dataInit, bool partial);
		Message(const std::vector<uint8_t>& dataInit);
		Message(const std::vector<uint8_t>& dataInit, bool partial);
		std::string text() const noexcept;
		size_t size() const noexcept;
	};
};

#endif
