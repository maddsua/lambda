#ifndef  __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__
#define  __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__

	#include <stdint.h>

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

#endif
