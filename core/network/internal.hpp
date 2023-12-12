
/*
	This header hides network-related functions that do not need to be exposed to the user
*/

#ifndef __LIB_MADDSUA_LAMBDA_INTERNAL__
#define __LIB_MADDSUA_LAMBDA_INTERNAL__

#include "./sysnetw.hpp"

namespace Lambda::Network {

	void setConnectionTimeouts(SOCKET hSocket, uint32_t timeoutsMs);

};

#endif
