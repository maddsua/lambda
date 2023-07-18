/*
	This header provides more or less a way to use sockets on bow Windows an Linux
	Should work in theory. If it does not, well, something is wrong 🗿🚬
*/

#ifndef __LAMBDA_NETWORK_TCPIP__
#define __LAMBDA_NETWORK_TCPIP__

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

	#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (-1)
	#endif

	#ifndef SOCKET_ERROR
	#define SOCKET_ERROR (-1)
	#endif

	#ifndef WSAETIMEDOUT
	#define WSAETIMEDOUT (ETIMEDOUT)
	#endif

	#define getAPIError() errno

	#define closesocket(socketHandle) close(socketHandle)
	#define SD_BOTH (SHUT_RDWR)

#endif

#endif
