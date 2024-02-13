#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__

#include "../network/tcp/connection.hpp"
#include "../http/transport.hpp"

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
			HTTP::Transport::TransportContext& transport;

		public:
			Writer(HTTP::Transport::TransportContext& tctx, const IncomingRequest& initRequest);
			void push(const EventMessage& event);
			bool connected() const noexcept;
			void close();
	};
};

#endif
