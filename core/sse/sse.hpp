#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_SIDE_EVENTS__

#include "../server/worker.hpp"
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

	struct WriterInit {
		const WorkerContext& workerctx;
		HTTP::Transport::TransportContext& transport;
		const IncomingRequest& requestEvent;
	};

	class Writer {
		private:
			const WorkerContext& m_worker;
			HTTP::Transport::TransportContext& m_transport;

		public:
			Writer(WriterInit init);
			void push(const EventMessage& event);
			bool connected() const noexcept;
			void close();
	};
};

#endif
