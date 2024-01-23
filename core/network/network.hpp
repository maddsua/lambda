#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <stdint.h>
#include <string>

#ifdef _WIN32

	#ifndef SOCKET
		typedef uint64_t SOCKET;
	#endif

	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET (-1ULL)
	#endif

#else

	#ifndef SOCKET
		typedef int SOCKET;
	#endif

	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET (-1)
	#endif

#endif

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
		uint32_t timeout;
		uint16_t hostPort;
	};

	namespace TCP {
		class Connection;
		class ListenSocket;
	};

};

#endif
