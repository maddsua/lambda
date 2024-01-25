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
		Unknown, TCP, UDP
	};

	struct Address {
		std::string hostname;
		uint16_t port = 0;
		ConnectionTransport transport = ConnectionTransport::Unknown;
	};

	struct ConnectionInfo {
		Address remoteAddr;
		uint32_t timeout = 0;
		uint16_t hostPort = 0;
	};

	namespace TCP {
		class Connection;
		class ListenSocket;
	};

};

#endif
