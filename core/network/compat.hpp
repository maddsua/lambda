#ifndef  __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__
#define  __LIB_MADDSUA_LAMBDA_NETWORK_COMPAT__

#include <stdint.h>

#ifndef SOCKET
	typedef uint64_t SOCKET;
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (-1UL)
#endif

#endif
