#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_WORKERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_WORKERS__

#include "../network/tcp/connection.hpp"

#include <thread>

namespace Lambda {

	struct WorkerContext {
		Net::TCP::Connection conn;
		std::thread worker;
		bool shutdownFlag = false;
		bool finished = false;
	};
};

#endif
