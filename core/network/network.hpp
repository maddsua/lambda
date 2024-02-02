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

	struct ConnectionInfo {
		Address remoteAddr;
		uint32_t timeout;
		uint16_t hostPort;
	};

	namespace TCP {
		class Connection;
		class ListenSocket;
	};
};

#endif
