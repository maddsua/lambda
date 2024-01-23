/*
	This header provides more or less a way to use sockets on bow Windows an Linux
	Should work in theory. If it does not, well, something is wrong 🗿🚬
*/

#ifndef __LIB_MADDSUA_LAMBDA_INTERNAL_NETWORK_TCPIP__
#define __LIB_MADDSUA_LAMBDA_INTERNAL_NETWORK_TCPIP__

	#include <stdint.h>

	#ifdef _WIN32

		#define WIN32_LEAN_AND_MEAN
		#include <winsock2.h>
		#include <ws2tcpip.h>
		#include <stdexcept>

		#define getAPIError() (GetLastError())

		#define LNE_ADDRINUSE	WSAEADDRINUSE
		#define LNE_TIMEDOUT	WSAETIMEDOUT

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

		#define closesocket(socketHandle) (close(socketHandle))
		#define SD_BOTH (SHUT_RDWR)

		#define LNE_ADDRINUSE	EADDRINUSE
		#define LNE_TIMEDOUT	ETIMEDOUT

	#endif

#endif
