#ifndef _LAMBDA_NETWORK_COMPAT_
#define _LAMBDA_NETWORK_COMPAT_

#include <stdint.h>

#ifndef SOCKET
	typedef uint64_t SOCKET;
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (-1)
#endif

#endif
