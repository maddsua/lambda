#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__
#define __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__

#include <cstdint>

namespace Lambda::Network {

	#ifdef _WIN32

		typedef uint64_t SockHandle;
		static const SockHandle invalid_socket = ~0;

	#else

		typedef int32_t SockHandle;
		static const SockHandle invalid_socket = -1;

	#endif
};

#endif
