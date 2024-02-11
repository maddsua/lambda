#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__

#include "../network/tcp/connection.hpp"

#include <string>
#include <map>
#include <optional>

namespace Lambda::SSE {

	struct EventMessage {
		std::string data;
		std::optional<std::string> event;
		std::optional<std::string> id;
		std::optional<uint32_t> retry;
	};

	class Writer {
		private:
			Network::TCP::Connection& conn;

		public:
			Writer(Network::TCP::Connection& connRef);
			void push(const EventMessage& event);
	};
};

#endif
