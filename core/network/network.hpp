#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <cstdint>
#include <string>
#include <optional>

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

		/*Address() {}
		Address(
			const std::string& hostInit,
			uint16_t portInit,
			ConnectionTransport transportInit
		) : hostname(hostInit), port(portInit), transport(transportInit) {}*/
	};

	struct ConnectionTimeouts {
		uint32_t rx = 0;
		uint32_t tx = 0;

		/*ConnectionTimeouts() {}
		ConnectionTimeouts(uint32_t value) : tx(value), rx(value) {}*/
	};

	struct ConnectionInfo {
		Address remoteAddr;
		ConnectionTimeouts timeouts;
		uint16_t hostPort = 0;

		/*ConnectionInfo() {}
		ConnectionInfo(
			const Address& remoteAddrInit,
			const ConnectionTimeouts& timeoutsInit,
			uint16_t hostPortInit
		) : remoteAddr(remoteAddrInit), timeouts(timeoutsInit), hostPort(hostPortInit) {}*/
	};

	namespace TCP {
		class Connection;
		class ListenSocket;
	};

};

#endif
