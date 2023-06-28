#ifndef __LAMBDA_SOCKETS_API__
#define __LAMBDA_SOCKETS_API__


#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	
#else

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>

	static const int INVALID_SOCKET = -1;
	static const int SOCKET_ERROR   = -1;

#endif

#endif
