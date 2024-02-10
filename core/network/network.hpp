#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include "../utils/utils.hpp"

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

	class NetworkError : public Lambda::Error {
		private:
			OS_Error m_os_error;

		public:
			NetworkError(const std::string& message) : Error(message) {}
			NetworkError(const std::string& message, const OS_Error& os_error)
				: Error(message + " (" + os_error.toString() + ")"), m_os_error(os_error) {}

			const OS_Error& osError() const noexcept { return this->m_os_error; }
	};

	#ifdef _WIN32
		typedef uint64_t SockHandle;
	#else
		typedef int32_t SockHandle;
	#endif
};

#endif
