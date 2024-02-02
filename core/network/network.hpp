#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <cstdint>
#include <string>

namespace Lambda::Network {

	enum struct ConnectionTransport {
		TCP, UDP
	};

	struct Address {
		std::string hostname;
		uint16_t port;
		ConnectionTransport transport;
	};

	struct ConnectionTimeouts {
		uint32_t receive = 15000;
		uint32_t send = 15000;
	};

	struct ConnectionInfo {
		Address remoteAddr;
		ConnectionTimeouts timeouts;
		uint16_t hostPort;
	};

	namespace TCP {
		class Connection;
		class ListenSocket;
	};

	enum struct SetTimeoutsDirection {
		Both, Send, Receive
	};
};

#endif
