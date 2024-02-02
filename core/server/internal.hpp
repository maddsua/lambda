#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_INTERNAL__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::Server {

	struct ReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
	};

};

#endif
