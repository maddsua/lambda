/*
	This header provides more or less a way to use sockets on bow Windows an Linux
	Should work in theory. If it does not, well, something is wrong 🗿🚬
*/

#ifndef __OCTOPUSS_INTERNAL_NETWORK_TCPIP__
#define __OCTOPUSS_INTERNAL_NETWORK_TCPIP__

#include <stdint.h>

#include "./compat.hpp"

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdexcept>

	#define getAPIError() GetLastError()

	inline bool wsaWakeUp() {

		static bool wsaInitCalled = false;
		if (wsaInitCalled) return false;
		wsaInitCalled = true;

		WSADATA initdata;
		if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
			throw std::runtime_error("WSA initialization failed: windows API error " + std::to_string(getAPIError()));

		return true;
	}

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
