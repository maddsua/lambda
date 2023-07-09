#ifndef __LAMBDA_NETWORK_APIS__
#define __LAMBDA_NETWORK_APIS__

#include <stdint.h>

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>

	#define getAPIError() GetLastError()
	
#else

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <cerrno>

	static const int INVALID_SOCKET = -1;
	static const int SOCKET_ERROR   = -1;

	#define getAPIError() errno

#endif

#endif
