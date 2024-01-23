#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <stdint.h>
#include <string>

namespace Lambda::Network {

	enum struct ConnectionTransport : int16_t {
		TCP, UDP
	};

	struct Address {
		ConnectionTransport transport;
		uint16_t port;
		std::string hostname;
	};

	#ifdef _WIN32
		bool wsaWakeUp();
	#endif

	void setConnectionTimeouts(SOCKET hSocket, uint32_t timeoutsMs);
};

#endif
