#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include "../utils/utils.hpp"

#include <cstdint>
#include <string>

namespace Lambda::Net {

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

	class NetworkError : public Lambda::Error {
		private:
			int32_t m_code;

		public:
			NetworkError(const std::string& message);
			NetworkError(const std::string& message, int32_t errorCode);

			const int32_t osError() const noexcept;
	};

	#ifdef _WIN32
		typedef uint64_t SockHandle;
	#else
		typedef int32_t SockHandle;
	#endif
};

#endif
