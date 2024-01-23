#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <stdint.h>
#include <string>

#include "./compat.hpp"

namespace Lambda::Network {

	enum struct ConnectionTransport : int16_t {
		TCP, UDP
	};

	struct Address {
		std::string hostname;
		uint16_t port;
		ConnectionTransport transport;
	};

	struct ConnectionInfo {
		Address remoteAddr;
		uint32_t connTimeout;
		uint16_t hostPort;
	};

	#ifdef _WIN32
		bool wsaWakeUp();
	#endif

	void setConnectionTimeouts(SOCKET hSocket, uint32_t timeoutsMs);
};

#endif
