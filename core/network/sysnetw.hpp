/*
	This header provides more or less a way to use sockets on bow Windows an Linux
	Should work in theory. If it does not, well, something is wrong 🗿🚬
*/

#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_INTERNAL__
#define __LIB_MADDSUA_LAMBDA_NETWORK_INTERNAL__

	#include <stdint.h>
	#include "../utils/utils.hpp"

	#ifdef _WIN32

		#define WIN32_LEAN_AND_MEAN
		#include <winsock2.h>
		#include <ws2tcpip.h>
		#include <stdexcept>

		inline bool wsaWakeUp() {

			static bool wsaInitCalled = false;
			if (wsaInitCalled) return false;
			wsaInitCalled = true;

			WSADATA initdata;
			if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
				throw std::runtime_error("WSA initialization failed:" + Lambda::OS_Error().toString());

			return true;
		}

	#else

		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netdb.h>
		#include <unistd.h>
		#include <arpa/inet.h>
		#include <cerrno>

		#define INVALID_SOCKET	(-1)

		#define closesocket(socketHandle) (close(socketHandle))
		#define SD_BOTH (SHUT_RDWR)

	#endif

#endif
