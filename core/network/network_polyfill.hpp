#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_POLYFILL__
#define __LIB_MADDSUA_LAMBDA_NETWORK_POLYFILL__

	#include <cstdint>

	#ifndef SOCKET
		typedef uint32_t SOCKET;
	#endif

	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET (-1)
	#endif

	#ifndef SOCKET_ERROR
		#define SOCKET_ERROR (-1)
	#endif

#endif
